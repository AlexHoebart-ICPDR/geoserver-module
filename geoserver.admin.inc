<?php

/**
 * ...
 */
function geoserver_content_type_settings($form, $form_state, $entity) {

  // TODO: check if GS is configured

  $sql = geoserver_generate_sql($entity->type);
  $settings = variable_get('geoserver_content_type_settings', array());
  $settings = isset($settings[$entity->type]) ? $settings[$entity->type] : array();
  $geometry_column = '';
  
  $fields = field_info_instances('node', $entity->type);

  foreach ($fields as $key => $field) {
    if ($field['widget']['module'] === 'postgis') {
      $geometry_column = $key.'_geometry';
    }
  }

  $form['content_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $entity->type,
  );
  $form['geometry_column'] = array(
    '#type' => 'hidden',
    '#default_value' => $geometry_column,
  );
  $form['title'] = array(
    '#type' => 'hidden',
    '#default_value' => $entity->name,
  );
  $form['layer'] = array(
    '#type' => 'textfield',
    '#title' => t('GeoServer layer'),
    '#default_value' => isset($settings['layer']) ? $settings['layer'] : $entity->type,
    '#rows' => 10,
    '#description' => t('Name of corresponding GeoServer layer.'),
  );
  $form['sql_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('SQL'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['sql_fieldset']['sql'] = array(
    '#type' => 'textarea',
    '#default_value' =>  $sql,
    '#rows' => 10,
    '#description' => t('Generated SQL.'),
  );
  $form['sld'] = array(
    '#type' => 'textfield',
    '#title' => t('GeoServer style'),
    '#default_value' => isset($settings['sld']) ? $settings['sld'] : $entity->type,
    '#rows' => 10,
    '#description' => t('Name of corresponding GeoServer style.'),
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
          'checked' => TRUE
        ),
      ),
    ),
  );
  $form['wms'] = array(
    '#type' => 'checkbox',
    '#title' => t('Provide an untiled OpenLayers WMS layer.'),
    '#default_value' => isset($settings['wms']) ? $settings['wms'] : 0,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#weight' => 10,
  );

  return $form;
}

function geoserver_content_type_settings_submit($form, &$form_state) {
  
  $setting_keys = array('layer', 'geometry_column', 'title', 'sld', 'gwc', 'gwc_srs', 'wfs', 'wms');
  $content_type = $form['content_type']['#value'];
  
  $settings = variable_get('geoserver_content_type_settings', array());
  $settings[$content_type] = array();
  
  foreach($form as $key => $field) {
    if (in_array($key, $setting_keys)) {
      $settings[$content_type][$key] = $field['#value'];
    }
  }
  
  variable_set('geoserver_content_type_settings', $settings);
}

function geoserver_generate_sql($type) {

  $connection = Database::getConnection();
  $prefix = $connection->tablePrefix();

  $sql = array(
      'fields' => array("SELECT node.nid as id, node.title"),
      'tables' => array("\nFROM ".$prefix."node"),
      'where' => array("\nWHERE node.type = '$type' and node.status = 1"),
  );

  $fields = field_info_instances('node', $type);

  foreach($fields as $field_name => $field) {

    $field = field_info_field($field_name);
    
    if ($field['cardinality'] === '1') {
      foreach ($field['storage']['details']['sql']['FIELD_LOAD_CURRENT'] as $table => $table_fields) {
        foreach ($table_fields as $table_field) {
          $sql['fields'][] = $table_field;
        }
        $sql['tables'][] = "\nLEFT JOIN $prefix$table ON node.nid = $table.entity_id";
      }
    }
  }

  return implode(', ', $sql['fields']).' '.implode(' ', $sql['tables']).' '.implode(' ', $sql['where']);
}