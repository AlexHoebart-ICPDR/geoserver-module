<?php

/**
 * @file
 * Provides OpenLayers layers.
 */

/**
 * Implements hook_openlayers_layers().
 */
function geoserver_openlayers_layers() {

  $olLayers = array();
  $gsLayers = geoserver_layers_load();
  $url = variable_get('geoserver_url', '');
  $workspace = variable_get('geoserver_workspace', '');

  foreach ($gsLayers as $gsLayer) {
    
    if (!isset($gsLayer->data['openlayers'])) {
      break; // Layer has no OpenLayers configuration.
    }

    if ($gsLayer->data['openlayers']['gwc']) {
      $olLayer = new stdClass();
      $olLayer->api_version = 1;
      $olLayer->name = $gsLayer->name . '_gwc';
      $olLayer->title = $gsLayer->title;
      $olLayer->description = t('Cached WMS layer of GeoServer layer %name',
          array('%name' => $gsLayer->name));
      $olLayer->data = array(
        'layer_type' => 'openlayers_layer_type_wms',
        'base_url' => $url . 'gwc/service/wms',
        'params' => array(
          'isBaseLayer' => 0,
          'buffer' => '2',
          'ratio' => '1.5',
          'singleTile' => 0,
        ),
        'options' => array(
          'srs' => $gsLayer->data['openlayers']['gwc_srs'],
          'TRANSPARENT' => 'true',
          'exceptions' => 'application/vnd.ogc.se_inimage',
          'format' => 'image/png',
          'layers' => $workspace . ':' . $gsLayer->name,
          'styles' => '',
        ),
      );
      $olLayers[$olLayer->name] = $olLayer;
    }

    if ($gsLayer->data['openlayers']['wfs']) {
      $olLayer = new stdClass();
      $olLayer->api_version = 1;
      $olLayer->name = $gsLayer->name . '_wfs';
      $olLayer->title = $gsLayer->title;
      $olLayer->description = t('WFS layer of GeoServer layer %name',
          array('%name' => $gsLayer->name));
      $olLayer->data = array(
        'layer_type' => 'openlayers_layer_type_geoserver_wfs',
        'sld' => $gsLayer->data['options']['sld'],
        'protocol' => array(
          'url' => $url . 'wfs',
          'typeName' => $gsLayer->name,
          'geometryName' => $gsLayer->data['options']['geometry_name'],
          'featureNS' => variable_get('geoserver_namespace', ''),
        ),
      );
      $olLayers[$olLayer->name] = $olLayer;
    }
  }

  return $olLayers;
}

