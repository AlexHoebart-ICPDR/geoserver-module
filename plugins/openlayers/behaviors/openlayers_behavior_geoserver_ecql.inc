<?php
/**
 * @file
 * Implementation of OpenLayers behavior.
 */

/**
 * Ctools plugin definition.
 */
function geoserver_openlayers_behavior_geoserver_ecql_openlayers_behaviors() {
  return array(
    'title' => t('CQL Filter'),
    'description' => t('Filter WMS and WFS layers using the Common Query Language (CQL).'),
    'type' => 'layer',
    'behavior' => array(
      'file' => 'openlayers_behavior_geoserver_ecql.inc',
      'class' => 'openlayers_behavior_geoserver_ecql',
      'parent' => 'openlayers_behavior',
    ),
  );
}

/**
 * Attribution Behavior
 */
class openlayers_behavior_geoserver_ecql extends openlayers_behavior {

  /**
   * Provide initial values for options.
   */
  function options_init() {
    return array(
    );
  }

  function options_form($defaults = array()) {

    $form = array();

    // Only prompt for GeoServer layers
    // TODO: Somehow add GeoServer WMS layers.
    $layers = array();
    foreach ($this->map['layers'] as $id => $name) {
      $layer = openlayers_layer_load($id);
      if ($layer->data['layer_type'] == 'openlayers_layer_type_geoserver_wfs') {
        $form[$name] = array(
          '#type' => 'textfield',
          '#title' => t('CQL Filter for %layer', array('%layer' => $name)),
          '#default_value' => (isset($defaults[$name])) ? $defaults[$name] : '',
        );
      }
    }

    $form['help'] = array(
      '#markup' => t('<p>See the <a href="@ecql">GeoServer CQL reference</a> on how to write filters. Use #example as placeholder for IDs of form field value. Placeholders get replaced on the change event of the form field.</p>',
        array('@ecql' => 'http://docs.geoserver.org/stable/en/user/filter/ecql_reference.html')),
    );

    return $form;
  }

  /**
   * Render.
   */
  function render(&$map) {
    drupal_add_js(drupal_get_path('module', 'geoserver') .
      '/plugins/openlayers/behaviors/openlayers_behavior_geoserver_ecql.js');
    return $this->options;
  }
}
