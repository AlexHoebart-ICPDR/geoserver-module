/**
 * @file
 * JS Implementation of OpenLayers behavior.
 */

/**
 * OpenLayers GeoServer ECQL Behavior
 */
Drupal.openlayers.addBehavior('openlayers_behavior_geoserver_ecql', function (data, options) {
  var map = data.openlayers;

  for (var name in options) {
    var layer = map.getLayersBy('drupalID', name)[0];
    if (typeof layer != 'undefined' && options[name] != '') {
      
      // Listen to change event for all given fields.
      var fields = options[name].match(/#(\w+)/g);
      for (var i in fields) {
        jQuery(fields[i]).change(function() {
          layer.protocol.params = OpenLayers.Util.extend(layer.protocol.params, {
            'cql_filter': options[name].replace(new RegExp(fields[i]), jQuery(this).val())
          });
          delete layer.protocol.params.bbox;
          layer.refresh({force: true});
        });
      }

      // Filter layer immediately if query doesn't contain fields.
      if (fields.length == 0) {
        layer.protocol.params = OpenLayers.Util.extend(layer.protocol.params, {
          'cql_filter': options[name]
        });
        delete layer.protocol.params.bbox;
      }
    }
  }
});
