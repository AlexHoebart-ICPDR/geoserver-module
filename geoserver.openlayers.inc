<?php

/**
 * @file
 * Provides OpenLayers layers.
 */

/**
 * Implements hook_openlayers_layers().
 */
function geoserver_openlayers_layers() {
  
  $url                   = variable_get('geoserver_url', '');
  $content_type_settings = variable_get('geoserver_content_type_settings', array());
  
  foreach($content_type_settings as $content_type => $setting) {
    
    if ($setting['gwc']) {
      $layer = new stdClass();
      $layer->api_version = 1;
      $layer->name = $content_type.'_gwc';
      $layer->title = $setting['title'];
      $layer->description = t('GeoWebCache layer for content type %name', array('%name' => $setting['title']));
      $layer->data = array(
        'baselayer' => FALSE,
        'projection' => array('4326'),
        'layer_type' => 'openlayers_layer_type_wms',
        'url' => trim($url, '/').'gwc/service/wms',
        'params' => array(
          'isBaseLayer' => FALSE,
          'layers' => $setting['layer'],
        ),
        'options' => array(
          'buffer' => 1,
        ),
      );
      $layers[$layer->name] = $layer;
    }
    
    if ($setting['wms']) {
      // TODO
    }
    
    if ($setting['wfs']) {
      // TODO
    }
    
  }
  
  return $layers;
}