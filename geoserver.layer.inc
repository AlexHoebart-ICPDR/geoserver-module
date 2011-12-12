<?php

/**
 * Provides a set of postgis_geometry with same geometry type and projection.
 */
class GeoServerLayer {

  private $type, $default, $custom, $remote, $workspace, $datastore;
  private $remote_loaded = FALSE;
  private $custom_loaded = FALSE;
  private $default_loaded = FALSE;

  /**
   * Creates an empty geometry set.
   *
   * @param string $type
   *   Entity type.
   */
  function __construct($node_type) {
    
    $this->type = $node_type;
    
    $this->workspace = variable_get('geoserver_workspace');
    $this->datastore = variable_get('geoserver_datastore');
  }
  
  public function get($field, $source = '') {
    if ((empty($source) && $this->remoteExists()) || $source == 'remote') {
      return $this->remoteExists() ? $this->remote->{$field} : '';
    }
    elseif ((empty($source) && $this->customExists()) || $source == 'custom') {
      return $this->customExists() ? $this->custom->{$field} : '';
    }
    else {
      $this->ensureDefaultLoaded();
      return $this->default->{$field};
    }
  }
  
  public function set($field, $value) {
    $this->custom->{$field} = $value;
  }
  
//  public function getDiff($field) {
//    $remote = $this->get($field, 'remote');
//    $custom = $this->get($field, 'custom');
//    $default = $this->get($field, 'default');
//    
//    if ($remote != $custom && $remote != $default && $custom != $default) {
//      return t(
//        'Default: %default, Custom: %custom, Remote: %remote', 
//        array('%default' => $default, '%custom' => $custom, '%remote' => $remote)
//      );
//    }
//    elseif ($default == $custom && $default != $remote) {
//      return t(
//        'Default and Custom: %default, Remote: %remote', 
//        array('%default' => $default, '%remote' => $remote)
//      );
//    }
//    elseif ($custom == $remote && $custom != $default) {
//      return t(
//        'Custom and Remote: %custom, Default: %default', 
//        array('%custom' => $custom, '%default' => $default)
//      );
//    }
//    elseif ($default == $remote && $default != $custom) {
//      return t(
//        'Default and Remote: %default, Custom: %custom', 
//        array('%default' => $default, '%custom' => $custom)
//      );
//    }
//  }
  
  private function ensureDefaultLoaded() {
    
    if (!$this->default_loaded) {
      
      $entity = node_type_load($this->type);
    
      $this->default->title = $entity->name;
      $this->default->style = $this->type;
      $this->default->sql = $this->getDefaultSQL();
      $this->default_loaded = TRUE;
    }
  }
  
  public function customExists() {
    if (!$this->custom_loaded) {
      $settings = variable_get('geoserver_entity_settings', array());
      if (isset($settings[$this->type])) {
        $this->custom_exists = TRUE;
        $this->custom = $settings[$this->type];
      } else {
        $this->custom_exists = FALSE;
      }
      $this->custom_loaded = TRUE;
    }
    return $this->custom_exists;
  }
  
  public function remoteExists() {
    if (!$this->remote_loaded) {
      $result = geoserver_get('rest/workspaces/' . $this->workspace . '/datastores/' . $this->datastore . '/featuretypes/'. $this->type .'.json');
      if ($result->code == 200) {
        $this->remote_exists = TRUE;
        $data = json_decode($result->data);
        $this->remote->title = $data->featureType->title;
        $this->remote->sql = $data->featureType->metadata->entry[2]->virtualTable->sql;
        $this->remote->minx = $data->featureType->nativeBoundingBox->minx;
        $this->remote->miny = $data->featureType->nativeBoundingBox->miny;
        $this->remote->maxx = $data->featureType->nativeBoundingBox->maxx;
        $this->remote->maxy = $data->featureType->nativeBoundingBox->maxy;
        $this->remote->srid = drupal_substr($data->featureType->srs, 5);
        $this->remote->geometry_name = $data->featureType->metadata->entry[2]->virtualTable->geometry->name;
        $this->remote->geometry_type = $data->featureType->metadata->entry[2]->virtualTable->geometry->type;
        
        // Fetch style name from layer configuration.
        $result = geoserver_get('rest/layers/' . $this->type .'.json');
        if ($result->code == 200) {
          $data = json_decode($result->data);
          $this->remote->style = $data->layer->defaultStyle->name;
        } else {
          $this->remote->style = FALSE;
        }
      } else {
        $this->remote_exists = FALSE;
      }
      $this->remote_loaded = TRUE;
    }
    return $this->remote_exists;
  }
  
  public function saveCustom() {
    
    // Save custom settings.
    $settings = variable_get('geoserver_entity_settings', array());
    $settings[$this->type] = $this->custom;
    variable_set('geoserver_entity_settings', $settings);
  }
  
  private function saveRemote() {
    
    $data = array(
      'featureType' => array(
        'name'       => $this->type,
        'nativeName' => $this->type,
        'namespace'  => array('name' => $this->workspace),
        'title'      => $this->custom->title,
        'nativeCRS'  => 'EPSG:' . $this->custom->srid,
        'srs'        => 'EPSG:' . $this->custom->srid,
        'nativeBoundingBox' => array(
          'minx' => $this->custom->minx,
          'miny' => $this->custom->miny,
          'maxx' => $this->custom->maxx,
          'maxy' => $this->custom->maxy,
        ),
        'latLonBoundingBox' => array(
          'minx' => $this->custom->minx,
          'miny' => $this->custom->miny,
          'maxx' => $this->custom->maxx,
          'maxy' => $this->custom->maxy,
        ),
        'projectionPolicy' => 'FORCE_DECLARED',
        'enabled' => TRUE,
        'metadata' => array(
          'entry' => array(
            array(
              '@key' => 'cachingEnabled', 
              '$' => 'false',
            ),
            array(
              '@key' => 'JDBC_VIRTUAL_TABLE',
              'virtualTable' => array(
                'name' => $this->type,
                'sql' => $this->custom->sql,
                'keyColumn' => 'id',
                'geometry' => array(
                  'name' => $this->custom->geometry_name,
                  'type' => $this->custom->geometry_type,
                  'srid' => $this->custom->srid,
                ),
              ),
            ),
          ),
        ),
        'store' => array(
          '@class' => 'dataStore',
          'name' => $this->datastore,
        ),
        'maxFeatures' => 0,
        'numDecimals' => 0,
        'attributes' => array(),
      ),
    );
    
    $url = 'rest/workspaces/' . $this->workspace . '/datastores/' . $this->datastore . '/featuretypes';
  
    if ($this->remoteExists()) {

      $result = geoserver_put($url . '/' . $this->type . '.json', $data);

      if ($result->code == 200) {
        $type = 'status';
        $message = t(
          'Updated GeoServer layer %title', 
          array('%title' => $this->custom->title)
        );
      }
      else {
        $type = 'error';
        $message = t(
          'Could not update GeoServer layer %title. Error (%code): %data.', 
          array(
            '%title' => $this->custom->title, 
            '%code' => $result->code, 
            '%data' => trim($result->data, ':'),
          )
        );
      }
    }
    else {

      $result = geoserver_post($url . '.json', $data);

      if ($result->code == 201) {
        $type = 'status';
        $message = t(
          'Created GeoServer layer %title', 
          array('%title' => $this->custom->title)
        );
      }
      else {
        $type = 'error';
        $message = t(
          'Could not create GeoServer layer %title. Error (%code): %data.', 
          array(
            '%title' => $this->custom->title, 
            '%code' => $result->code, 
            '%data' => trim($result->data, ':'),
          )
        );
      }
    }
    drupal_set_message($message, $type);
  }
  
  /**
   * Generate content type specific SQL for GeoServer SQL layer.
   */
  private function getDefaultSQL() {

    $connection = Database::getConnection();
    $prefix = $connection->tablePrefix();

    $sql = array(
        'fields' => array("SELECT node.nid as id, node.title"),
        'tables' => array("\nFROM $prefix node"),
        'where' => array("\nWHERE node.type = '$this->type' and node.status = 1"),
    );

    $fields = field_info_instances('node', $this->type);

    foreach ($fields as $field_name => $field) {

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

    return implode(', ', $sql['fields']) . ' ' . implode(' ', $sql['tables']) . ' ' . implode(' ', $sql['where']);
  }
}