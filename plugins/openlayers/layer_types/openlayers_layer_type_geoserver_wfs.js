
/**
 * @file
 * Layer handler for GeoServer WFS layers
 */

/**
 * Openlayer layer handler for GeoServer WFS layer
 */
Drupal.openlayers.layer.openlayers_layer_type_geoserver_wfs = function(title, map, options) {

  var layer = new OpenLayers.Layer.Vector(title, {
    drupalID: options.drupalID,
    strategies: [new OpenLayers.Strategy.BBOX()],
    projection: 'EPSG:'+map.projection,
    buffer: 0,
    styleMap: new OpenLayers.StyleMap(),
    protocol: new OpenLayers.Protocol.Script({
      url: options.protocol.url,
      callbackKey: 'format_options',
      callbackPrefix: 'callback:',
      params: {
        service: 'WFS',
        version: '1.0.0',
        request: 'GetFeature',
        typeName: options.protocol.featureNS+':'+options.protocol.typeName,
        outputFormat: 'json',
        srsName: 'EPSG:'+map.projection
      },
      filterToParams: function(filter, params) {
        if (filter.type === OpenLayers.Filter.Spatial.BBOX) {
          params.bbox = filter.value.toArray();
          if (filter.projection) {
            params.bbox.push(filter.projection.getCode());
          }
        }
        return params;
      }
    })
  });
  
  // Apply GeoServer SLD.
  var sld = new OpenLayers.Format.SLD().read(options.sld);
  if (sld.namedLayers[options.protocol.typeName]) {
    jQuery.each(sld.namedLayers[options.protocol.typeName].userStyles, function(index, style) {

      // Set cursor to pointer.
      style.defaultsPerSymbolizer = false;
      style.defaultStyle.cursor = 'pointer';

      // Prepend path of external graphics with geoserver_url if path is relative.
      // This way OpenLayers can find graphics that are stored and used inside GeoServer.
      jQuery.each(style.rules, function(index, rule) {
        if (rule.symbolizer.Point &&
            rule.symbolizer.Point.externalGraphic &&
            rule.symbolizer.Point.externalGraphic.substr(0, 4) != 'http' &&
            rule.symbolizer.Point.externalGraphic.substr(0, 1) != '/') {
          rule.symbolizer.Point.externalGraphic = options.geoserver_url+
            'styles/'+rule.symbolizer.Point.externalGraphic;
        }
      });

      // Apply style to layer.
      layer.styleMap.styles[style.description] = style;
      layer.redraw();
    });
  }
  
  return layer;
};

