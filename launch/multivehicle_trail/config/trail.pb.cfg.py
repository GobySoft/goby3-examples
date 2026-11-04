#!/usr/bin/env python3

# Prints the configuration for one application of the multivehicle_trail mission.
# Usage: trail.pb.cfg.py {topside|usv|auv} app_name
# Environment: trail_n_auvs (usv, auv), trail_auv_index (auv only), trail_map_tiles (topside)

import os
import sys
from string import Template

def fail(message):
    sys.exit('multivehicle_trail configuration: ' + message)

if len(sys.argv) != 3:
    fail('usage: trail.pb.cfg.py {topside|usv|auv} app_name')
role, app = sys.argv[1:]

config_dir = os.path.dirname(os.path.realpath(__file__))
examples_lib_dir = os.path.normpath(config_dir + '/../../../build/lib')

# Buzzard's Bay, where the OpenSeaMap overlay has plenty of buoys and lights to show
lat_origin, lon_origin = 41.592344, -70.711260
warp = 5 # must match MOOSTimeWarp

def env_int(name):
    try:
        return int(os.environ[name])
    except (KeyError, ValueError):
        fail('set the {} environment variable (see all.launch)'.format(name))

n_auvs = env_int('trail_n_auvs') if role != 'topside' else 0
auv_index = env_int('trail_auv_index') if role == 'auv' else None

# Vehicle ids: topside 0, USV 1, AUVs from 2. Modem ids are vehicle id + 1 (0 is broadcast),
# with the satellite link on subnet 0x0000 and the acoustic link on subnet 0x0100.
usv_id = 1
vehicle_id = {'topside': 0, 'usv': usv_id, 'auv': 2 + (auv_index or 0)}[role]
platform = 'auv{}'.format(auv_index) if role == 'auv' else role

def satellite_modem_id(vid): return vid + 1
def acomms_modem_id(vid): return 0x100 + vid + 1
auv_modem_ids = [acomms_modem_id(2 + i) for i in range(n_auvs)]

app_block = Template('''app {
  name: "$app"
  glog_config { tty_verbosity: QUIET }
  simulation { time { use_sim_time: true warp_factor: $warp reference_microtime: 0 } }
  geodesy { lat_origin: $lat lon_origin: $lon }
}
interprocess { platform: "$platform" }
''').substitute(app=app, warp=warp, lat=lat_origin, lon=lon_origin, platform=platform)

def link(modem_id, port, slots):
    return Template('''  link {
    modem_id: $modem_id
    subnet_mask: 0xFF00
    driver {
      driver_type: DRIVER_UDP_MULTICAST
      [goby.acomms.udp_multicast.protobuf.config] {
        multicast_address: "239.142.0.2"
        multicast_port: $port
        max_frame_size: 1400
      }
    }
    mac { type: MAC_FIXED_DECENTRALIZED $slots }
  }
''').substitute(modem_id=modem_id, port=port, slots=slots)

def satellite_link():
    slots = ' '.join('slot {{ src: {} slot_seconds: 1 max_frame_bytes: 128 }}'.format(
        satellite_modem_id(v)) for v in (0, usv_id))
    return link(satellite_modem_id(vehicle_id), 54500, slots)

def acomms_link():
    # the USV gets a slot before each AUV's so it is heard as often as all the AUVs together
    slots = ' '.join('slot {{ src: {} slot_seconds: 10 max_frame_bytes: 128 }} '
                     'slot {{ src: {} slot_seconds: 10 max_frame_bytes: 128 }}'.format(
                         acomms_modem_id(usv_id), auv) for auv in auv_modem_ids)
    return link(acomms_modem_id(vehicle_id), 54501, slots)

def gobyd():
    links = {'topside': [satellite_link()],
             'usv': [satellite_link(), acomms_link()],
             'auv': [acomms_link()]}[role]
    return app_block + 'intervehicle {\n' + ''.join(links) + '}\n'

def simulator_port(): return 55000 + vehicle_id
def moos_port(): return 9000 + vehicle_id

def frontseat():
    return app_block + Template('''frontseat_cfg {
  require_helm: true
  helm_running_timeout: 10
  frontseat_connected_timeout: 10
  status_period: 5
  exit_on_error: false
  type: $type
  [goby.middleware.frontseat.protobuf.basic_simulator_config] {
    tcp_address: "127.0.0.1"
    tcp_port: $sim_port
    start {
      lat: $lat
      lon: $lon
      duration: 0
      control_freq: 1
      vehicle { accel: 1.0 hdg_rate: 80 z_rate: 2 }
    }
  }
}
[goby.moos.protobuf.moos_helm] {
  server: "127.0.0.1"
  port: $moos_port
  use_binary_protobuf: false
  moos_parser_technique: TECHNIQUE_PREFIXED_PROTOBUF_TEXT_FORMAT
}
''').substitute(type=role.upper(), sim_port=simulator_port(), moos_port=moos_port(),
                lat=lat_origin, lon=lon_origin)

# trail_map_tiles picks the Map tab's background; unset uses Liaison's default of OpenStreetMap
# with the OpenSeaMap seamarks
openseamap = """  tile_layer {
    url_template: "https://tiles.openseamap.org/seamark/{z}/{x}/{y}.png"
    attribution: "&copy; <a href=\\"https://www.openseamap.org\\">OpenSeaMap</a> contributors"
    max_zoom: 18
    overlay: true
  }
"""
map_tiles = {
    '': '',
    # Esri's Ocean basemap: shaded bathymetry from GEBCO and NOAA, under the seamarks
    'bathymetry': """  tile_layer {
    url_template: "https://server.arcgisonline.com/ArcGIS/rest/services/Ocean/World_Ocean_Base/MapServer/tile/{z}/{y}/{x}"
    attribution: "Tiles &copy; Esri &mdash; Sources: GEBCO, NOAA, CHS, OSU, UNH, CSUMB, National Geographic, DeLorme, NAVTEQ, and Esri"
    max_zoom: 16
  }
""" + openseamap,
    # no tiles at all, as on a vessel with no internet connection
    'none': '  blank_background: true\n',
}

def liaison():
    tiles = os.environ.get('trail_map_tiles', '')
    if tiles not in map_tiles:
        fail('trail_map_tiles must be one of: bathymetry, none')
    return app_block + Template('''http_address: "0.0.0.0"
http_port: 50000
load_shared_library: "$lib"
pb_map_config {
  stale_timeout_seconds: 30
$tiles}
''').substitute(lib=examples_lib_dir + '/libgoby3_example_trail_messages.so', tiles=map_tiles[tiles])

def manager():
    if role == 'topside':
        return app_block + 'usv_modem_id: {}\n'.format(satellite_modem_id(usv_id))
    if role == 'usv':
        return app_block + 'vehicle_id: {}\n'.format(vehicle_id) + ''.join(
            'auv_modem_id: {}\n'.format(m) for m in auv_modem_ids)
    return app_block + 'vehicle_id: {}\nusv_modem_id: {}\n'.format(
        vehicle_id, acomms_modem_id(usv_id))

def moos_gateway():
    return app_block + 'moos {{ server: "localhost" port: {} }}\npoll_frequency: 10\n'.format(
        moos_port())

def moos():
    return Template(open(config_dir + '/vehicle.moos.in').read()).substitute(
        moos_port=moos_port(), community=platform, warp=warp, lat=lat_origin, lon=lon_origin,
        bhv_file='/tmp/multivehicle_trail/{}.bhv'.format(platform),
        speed_max=4 if role == 'auv' else 2)

def bhv():
    if role == 'usv':
        return open(config_dir + '/usv.bhv').read()
    # fan the AUVs out behind the USV from 90 to 270 degrees, each 10 m deeper than the last
    trail_angle = 180 if n_auvs == 1 else 90 + 180 * auv_index // (n_auvs - 1)
    return Template(open(config_dir + '/auv.bhv.in').read()).substitute(
        trail_angle=trail_angle, deploy_depth=10 + 10 * auv_index)

generators = {
    'gobyd': gobyd,
    'goby_frontseat_interface': frontseat,
    'goby_liaison': liaison,
    'goby3_example_trail_topside_manager': manager,
    'goby3_example_trail_usv_manager': manager,
    'goby3_example_trail_auv_manager': manager,
    'goby_moos_gateway': moos_gateway,
    'moos': moos,
    'bhv': bhv,
    'simulator_port': lambda: str(simulator_port()),
}
if app not in generators:
    fail('no configuration for ' + app)
print(generators[app]())
