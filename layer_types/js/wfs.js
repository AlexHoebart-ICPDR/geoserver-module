
/**
 * @file
 * Layer handler for GeoServer WFS layers
 */

/**
 * Openlayer layer handler for GeoServer WFS layer
 */
Drupal.openlayers.layer.geoserver_wfs = function(title, map, options) {

  var layer = new OpenLayers.Layer.Vector(title, {
    drupalID: options.drupalID,
    strategies: [new OpenLayers.Strategy.BBOX()],
    projection: 'EPSG:'+map.projection,
    buffer: 0,
    styleMap: new OpenLayers.StyleMap(),
    protocol: new OpenLayers.Protocol.WFS({
      version: "1.1.0",
      url: options.protocol.url,
      featureType: options.protocol.typeName,
      geometryName: options.protocol.geometryName,
      featureNS: options.protocol.featureNS,
      srsName: 'EPSG:'+map.projection
    })
  });
  
  if (typeof options.sld === 'string') {
    OpenLayers.Request.GET({
      url: options.sld,
      success: function(request) {
        var sld = new OpenLayers.Format.SLD().read(request.responseXML || request.responseText);
        if (sld.namedLayers[options.protocol.typeName]) {
          jQuery.each(sld.namedLayers[options.protocol.typeName].userStyles, function(index, style) {
            style.defaultsPerSymbolizer = false;
            style.defaultStyle.cursor = 'pointer';
            layer.styleMap.styles[style.description] = style;
            layer.redraw();
          });
        }
      }
    });
  }
  
  return layer;
};