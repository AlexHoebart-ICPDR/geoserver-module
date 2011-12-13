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
  $geoserver_entity_settings = variable_get('geoserver_entity_settings', array());

  foreach ($geoserver_entity_settings as $entity => $settings) {

    if ($settings->gwc) {
      $layer = new stdClass();
      $layer->api_version = 1;
      $layer->name = $entity . '_gwc';
      $layer->title = $settings->title;
      $layer->description = t('GeoWebCache layer for content type %name',
          array('%name' => $settings->title));
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
          'srs' => $settings->gwc_srs,
          'TRANSPARENT' => 'true',
          'exceptions' => 'application/vnd.ogc.se_inimage',
          'format' => 'image/png',
          'layers' => $workspace . ':' . $entity,
          'styles' => '',
        ),
      );
      $layers[$layer->name] = $layer;
    }

    if ($settings->wfs) {
      $layer = new stdClass();
      $layer->api_version = 1;
      $layer->name = $entity . '_wfs';
      $layer->title = $settings->title;
      $layer->description = t('Vector layer of content type %name',
          array('%name' => $settings->title));
      $layer->data = array(
        'layer_type' => 'openlayers_layer_type_geoserver_wfs',
        'sld' => $settings->sld,
        'protocol' => array(
          'url' => $url . 'wfs',
          'typeName' => $entity,
          'geometryName' => $settings->geometry_name,
          'featureNS' => variable_get('geoserver_namespace', ''),
        ),
      );
      $layers[$layer->name] = $layer;
    }
  }

  return $layers;
}

