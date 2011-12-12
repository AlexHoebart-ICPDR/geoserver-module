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
    drupal_set_message(t('Found GeoServer layer %title', array('%title' => $layer->get('title'))));
  } 
  elseif (empty($form_state['input'])) {
    drupal_set_message(t('GeoServer layer not found.'), 'warning');
  }
  
  $form['entity_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $entity->type,
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $layer->get('title'),
    '#description' => 'Layer title.',
  );
  $form['srid'] = array(
    '#type' => 'textfield',
    '#title' => t('Projection'),
    '#default_value' => $layer->get('srid'),
    '#description' => t('TODO: fetch from data'),
  );
  $form['sql'] = array(
    '#type' => 'textarea',
    '#title' => t('SQL'),
    '#default_value' =>  $layer->get('sql'),
    '#rows' => 8,
    '#description' => t('Generated SQL.'),
  );
  $form['bounding_box'] = array(
    '#type' => 'fieldset',
    '#title' => t('Bounding Box'),
    '#description' => t('TODO: fetch from data'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['bounding_box']['minx'] = array(
    '#type' => 'textfield',
    '#title' => t('Min Y'),
    '#size' => 4,
    '#default_value' => $layer->get('minx'),
  );
  $form['bounding_box']['miny'] = array(
    '#type' => 'textfield',
    '#title' => t('Min Y'),
    '#size' => 4,
    '#default_value' => $layer->get('miny'),
  );
  $form['bounding_box']['maxx'] = array(
    '#type' => 'textfield',
    '#title' => t('Max X'),
    '#size' => 4,
    '#default_value' => $layer->get('maxx'),
  );
  $form['bounding_box']['maxy'] = array(
    '#type' => 'textfield',
    '#title' => t('Max Y'),
    '#size' => 4,
    '#default_value' => $layer->get('maxy'),
  );
  
  $form['geometry_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Geometry name'),
    '#default_value' => $layer->get('geometry_name'),
    '#description' => t('...'),
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
    '#description' => '...',
  );
  
  $options = array('');
  $result = geoserver_get('rest/styles.json');
  if ($result->code == 200) {
    $data = json_decode($result->data);
    foreach ($data->styles->style as $style) {
      $options[$style->name] = $style->name;
    }
  }
  $form['styles'] = array(
    '#type' => 'select',
    '#title' => t('Styles'),
    '#default_value' => $layer->get('style'),
    '#options' => $options,
    '#description' => $layer->get('style'),
  );
  
  $form['style_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Style'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['style_fieldset']['style'] = array(
    '#type' => 'textarea',
    '#default_value' =>  '',
    '#rows' => 10,
    '#description' => t('SLD style.'),
  );
  $form['wfs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide an OpenLayers WFS layer.'),
    '#default_value' => isset($settings['wfs']) ? $settings['wfs'] : 0,
  );
  $form['gwc'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide a cached OpenLayers WMS layer.'),
    '#default_value' => isset($settings['gwc']) ? $settings['gwc'] : 0,
  );
  $form['gwc_srs'] = array(
    '#type' => 'select',
    '#title' => t('Projection'),
    '#options' => array(
      'EPSG:4326' => '4326',
      'EPSG:900913' => '900913',
    ),
    '#default_value' => isset($settings['gwc_srs']) ? $settings['gwc_srs'] : 'EPSG:4326',
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
    '#default_value' => isset($settings['wms']) ? $settings['wms'] : 0,
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
  $layer->saveCustom();
  if ($form['publish']['#value']) {
    $layer->saveRemote();
  }
}



