<?php

/**
 * @file
 * Administration interface.
 */

/**
 * Content type settings form.
 */
function geoserver_content_type_settings($form, $form_state, $entity) {
  
  $layer = new GeoServerLayer($entity->type);
  
  // Display message about status of the GeoServer layer.
  if ($layer->remoteExists() && empty($form_state['input'])) {
    drupal_set_message(t('Found remote GeoServer settings.'));
  } 
  elseif ($layer->customExists() && empty($form_state['input'])) {
    drupal_set_message(t('Found custom GeoServer settings.'));
  }
  elseif (empty($form_state['input'])) {
    drupal_set_message(t('Found default GeoServer settings.'));
  }
  
  $form['entity_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $entity->type,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $layer->get('title'),
  );
  $form['srid'] = array(
    '#type' => 'textfield',
    '#title' => t('Projection'),
    '#default_value' => $layer->get('srid'),
  );
  $form['geometry_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Geometry name'),
    '#default_value' => $layer->get('geometry_name'),
  );
  $form['geometry_type'] = array(
    '#type' => 'select',
    '#title' => t('Geometry type'),
    '#default_value' => $layer->get('geometry_type'),
    '#options' => array(
      'Geometry' => 'Geometry',
      'GeometryCollection' => 'GeometryCollection',
      'Point' => 'Point',
      'MultiPoint' => 'MultiPoint',
      'LineString' => 'LineString',
      'MultiLineString' => 'MultiLineString',
      'Polygon' => 'Polygon',
      'MultiPolygon' => 'MultiPolygon',
    ),
  );
  
  $form['bounding_box'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bounding Box'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['bounding_box']['minx'] = array(
    '#type' => 'textfield',
    '#title' => t('Min X'),
    '#default_value' => $layer->get('minx'),
  );
  $form['bounding_box']['miny'] = array(
    '#type' => 'textfield',
    '#title' => t('Min Y'),
    '#default_value' => $layer->get('miny'),
  );
  $form['bounding_box']['maxx'] = array(
    '#type' => 'textfield',
    '#title' => t('Max X'),
    '#default_value' => $layer->get('maxx'),
  );
  $form['bounding_box']['maxy'] = array(
    '#type' => 'textfield',
    '#title' => t('Max Y'),
    '#default_value' => $layer->get('maxy'),
  );
  
  $options = array('');
  $result = geoserver_get('rest/styles.json');
  if ($result->code == 200) {
    $data = json_decode($result->data);
    foreach ($data->styles->style as $style) {
      $options[$style->name] = $style->name;
    }
  } 
  else {
    $options[$layer->get('style')] = $layer->get('style');
  }
  $form['style'] = array(
    '#type' => 'fieldset',
    '#title' => t('Style'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['style']['name'] = array(
    '#type' => 'select',
    '#title' => t('Style'),
    '#default_value' => $layer->get('style'),
    '#options' => $options,
    '#ajax' => array(
      'callback' => 'geoserver_update_style_sld',
      'wrapper' => 'geoserver_style_sld'
    ),
  );
  $form['style']['sld'] = array(
    '#type' => 'textarea',
    '#title' => t('SLD'),
    '#default_value' =>  $layer->get('sld'),
    '#rows' => 10,
    '#prefix' => '<div id="geoserver_style_sld">',
    '#suffix' => '</div>',
  );
  
  $form['sql'] = array(
    '#type' => 'textarea',
    '#title' => t('SQL'),
    '#default_value' =>  $layer->get('sql'),
    '#rows' => 10,
  );
  
  $form['wfs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide an OpenLayers WFS layer.'),
    '#default_value' => $layer->get('wfs', 'custom'), 
  );
  $form['gwc'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide a cached OpenLayers WMS layer.'),
    '#default_value' => $layer->get('gwc', 'custom'),
  );
  $form['gwc_srs'] = array(
    '#type' => 'select',
    '#title' => t('Projection'),
    '#options' => array(
      'EPSG:4326' => '4326',
      'EPSG:900913' => '900913',
    ),
    '#default_value' => $layer->get('gwc_srs', 'custom'),
    '#states' => array(
      'visible' => array(
        ':input[name="gwc"]' => array(
          'checked' => TRUE,
        ),
      ),
    ),
  );
  $form['wms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide an untiled OpenLayers WMS layer.'),
    '#default_value' => $layer->get('wms', 'custom'),
  );
  $form['publish'] = array(
    '#type' => 'checkbox',
    '#title' => t('Update GeoServer configuration.'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save settings'),
    '#weight' => 10,
  );

  return $form;
}

function geoserver_update_style_sld($form, $form_state) {
  
  $result = geoserver_get('rest/styles/' . $form['style']['name']['#value'] .'.sld');
            
  if ($result->code == 200) {
    $sld = new DOMDocument();
    $sld->formatOutput = TRUE;
    $sld->loadXML($result->data);
  }
  $form['style']['sld']['#value'] = $result->code == 200 ? $sld->saveXML() : '';
  
  return $form['style']['sld'];
}

/**
 * Save content type settings.
 */
function geoserver_content_type_settings_submit($form, &$form_state) {
  $layer = new GeoServerLayer($form['entity_type']['#value']);
  $layer->set('title', $form['title']['#value']);
  $layer->set('sql', $form['sql']['#value']);
  $layer->set('minx', $form['bounding_box']['minx']['#value']);
  $layer->set('miny', $form['bounding_box']['miny']['#value']);
  $layer->set('maxx', $form['bounding_box']['maxx']['#value']);
  $layer->set('maxy', $form['bounding_box']['maxy']['#value']);
  $layer->set('geometry_name', $form['geometry_name']['#value']);
  $layer->set('geometry_type', $form['geometry_type']['#value']);
  $layer->set('srid', $form['srid']['#value']);
  $layer->set('style', $form['style']['name']['#value']);
  $layer->set('sld', $form['style']['sld']['#value']);
  $layer->set('wfs', $form['wfs']['#value']);
  $layer->set('gwc', $form['gwc']['#value']);
  $layer->set('gwc_srs', $form['gwc_srs']['#value']);
  $layer->set('wms', $form['wms']['#value']);
  $layer->saveCustom();
  if ($form['publish']['#value']) {
    $layer->saveRemote();
  }
}



