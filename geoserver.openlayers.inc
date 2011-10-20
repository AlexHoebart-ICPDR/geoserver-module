<?php

/**
 * @file
 * Provides OpenLayers layers.
 */

/**
 * Implementation of hook_openlayers_layer_types()
 */
function geoserver_openlayers_layer_types() {
  return array(
    'geoserver_layer_type_wfs' => array(
      'title' => t('GeoServer WFS'),
      'description' => t('A GeoServer WFS layer.'),
      'layer_type' => array(
        'path' => drupal_get_path('module', 'geoserver') .'/layer_types',
        'file' => 'wfs.inc',
        'class' => 'geoserver_layer_type_wfs',
        'parent' => 'openlayers_layer_type',
      ),
    )
  );
}

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
        'layer_type' => 'openlayers_layer_type_wms',
        'base_url' => trim($url, '/').'/gwc/service/wms',
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
          'layers' => variable_get('geoserver_workspace', '').':'.$setting['layer'],
          'styles' => '',
        ),
      );
      $layers[$layer->name] = $layer;
    }
    
    if ($setting['wms']) {
      // TODO
    }
    
    if ($setting['wfs']) {
      $layer = new stdClass();
      $layer->api_version = 1;
      $layer->name = $content_type.'_wfs';
      $layer->title = $setting['title'];
      $layer->description = t('GeoServer WFS layer for content type %name', array('%name' => $setting['title']));
      $layer->data = array(
        'layer_type' => 'geoserver_layer_type_wfs',
        'sld' => trim($url, '/').'/styles/'.$setting['sld'].'.sld',
        'protocol' => array(
          'url' => trim($url, '/').'/wfs',
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