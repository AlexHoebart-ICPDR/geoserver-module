<?php

/**
 * ...
 */
function geoserver_content_type_settings($form, $form_state, $entity) {

  $sql = geoserver_generate_sql($entity->type);

  $form['sql'] = array(
    '#type' => 'textarea',
    '#title' => t('SQL'),
    '#default_value' =>  $sql,
    '#rows' => 10,
    '#description' => t('Generated SQL.'),
  );

  $form['geoserver_layer'] = array(
    '#type' => 'textfield',
    '#title' => t('GeoServer layer'),
    '#default_value' =>  $entity->type,
    '#rows' => 10,
    '#description' => t('Name of corresponding GeoServer layer.'),
  );

  return $form;
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