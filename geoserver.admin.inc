<?php

/**
 * ...
 */
function geoserver_content_type_settings($form, $form_state, $entity) {

  $sql = geoserver_generate_sql($entity->type);
  $settings = variable_get('geoserver_content_type_settings', array());
  $settings = isset($settings[$entity->type]) ? $settings[$entity->type] : array();

  $form['sql'] = array(
    '#type' => 'textarea',
    '#title' => t('SQL'),
    '#default_value' =>  $sql,
    '#rows' => 10,
    '#description' => t('Generated SQL.'),
  );
  $form['content_type'] = array(
    '#type' => 'hidden',
    '#default_value' => $entity->type,
  );
  $form['layer'] = array(
    '#type' => 'textfield',
    '#title' => t('GeoServer layer'),
    '#default_value' => isset($settings['layer']) ? $settings['layer'] : 
        variable_get('geoserver_workspace', '').':'.$entity->type,
    '#rows' => 10,
    '#description' => t('Name of corresponding GeoServer layer.'),
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
  
  $setting_keys = array('layer', 'gwc', 'wfs', 'wms');
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
  
  /**
   * TODO:
   * - handle n:m fields
   */

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

    foreach ($field['storage']['details']['sql']['FIELD_LOAD_CURRENT'] as $table => $table_fields) {
      foreach ($table_fields as $table_field) {
        $sql['fields'][] = $table_field;
      }
      $sql['tables'][] = "\nLEFT JOIN $prefix$table ON node.nid = $table.entity_id";
    }
  }

  return implode(', ', $sql['fields']).' '.implode(' ', $sql['tables']).' '.implode(' ', $sql['where']);
}