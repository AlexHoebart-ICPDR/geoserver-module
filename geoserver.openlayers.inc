<?php

/**
 * @file
 * Provides OpenLayers layers.
 */

/**
 * Implements hook_openlayers_layer_types().
 */
function geoserver_openlayers_layer_types() {
  ctools_include('plugins');
  $layer_types = ctools_get_plugins('geoserver', 'layer_types');
  return $layer_types;
}

/**
 * Implements hook_openlayers_layers().
 */
function geoserver_openlayers_layers() {

  $layers = array();
  $url = variable_get('geoserver_url', '');
  $workspace = variable_get('geoserver_workspace', '');
  $content_type_settings = variable_get('geoserver_content_type_settings', array());

  foreach ($content_type_settings as $content_type => $setting) {

    if ($setting['gwc']) {
      $layer = new stdClass();
      $layer->api_version = 1;
      $layer->name = $content_type . '_gwc';
      $layer->title = $setting['title'];
      $layer->description = t('GeoWebCache layer for content type %name',
          array('%name' => $setting['title']));
      $layer->data = array(
        'layer_type' => 'openlayers_layer_type_wms',
        'base_url' => $url . 'gwc/service/wms',
        'params' => array(
          'isBaseLayer' => 0,
          'buffer' => '2',
          'ratio' => '1.5',
          'singleTile' => 0,
        ),
        'options' => array(
          'srs' => $setting['gwc_srs'],
          'TRANSPARENT' => 'true',
          'exceptions' => 'application/vnd.ogc.se_inimage',
          'format' => 'image/png',
          'layers' => $workspace . ':' . $setting['layer'],
          'styles' => '',
        ),
      );
      $layers[$layer->name] = $layer;
    }

    if ($setting['wfs']) {
      $layer = new stdClass();
      $layer->api_version = 1;
      $layer->name = $content_type . '_wfs';
      $layer->title = $setting['title'];
      $layer->description = t('Vector layer of content type %name',
          array('%name' => $setting['title']));
      $layer->data = array(
        'layer_type' => 'openlayers_layer_type_geoserver_wfs',
        'sld' => $url . 'styles/' . $setting['sld'] . '.sld',
        'protocol' => array(
          'url' => $url . 'wfs',
          'typeName' => $setting['layer'],
          'geometryName' => $setting['geometry_column'],
          'featureNS' => variable_get('geoserver_namespace', ''),
        ),
      );
      $layers[$layer->name] = $layer;
    }
  }

  return $layers;
}

