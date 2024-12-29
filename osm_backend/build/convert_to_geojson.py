import osmium as osm
import geopandas as gpd
from shapely.geometry import Point, LineString, Polygon

class OSMHandler(osm.SimpleHandler):
    def __init__(self):
        super(OSMHandler, self).__init__()
        self.nodes = {}
        self.features = []

    def node(self, n):
        self.nodes[n.id] = (n.location.lon, n.location.lat)

    def way(self, w):
        if not w.is_closed():
            return
        
        coords = [self.nodes[nid] for nid in w.nodes if nid in self.nodes]
        if len(coords) < 3:
            return
        
        polygon = Polygon(coords)
        properties = {'id': w.id}
        feature = {'type': 'Feature', 'geometry': polygon, 'properties': properties}
        self.features.append(feature)

def convert_osm_to_geojson(osm_file_path, geojson_output_path):
    handler = OSMHandler()
    handler.apply_file(osm_file_path)

    gdf = gpd.GeoDataFrame(handler.features)
    gdf.crs = "EPSG:4326"
    gdf.to_file(geojson_output_path, driver='GeoJSON')

# Example usage
convert_osm_to_geojson('map.osm', 'map.geojson')