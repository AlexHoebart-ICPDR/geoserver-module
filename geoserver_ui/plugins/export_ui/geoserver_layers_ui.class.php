<?php
/**
 * @file
 */

class geoserver_layers_ui extends ctools_export_ui {

  /**
   * Build a row based on the item.
   *
   * By default all of the rows are placed into a table by the render
   * method, so this is building up a row suitable for theme('table').
   * This doesn't have to be true if you override both.
   */
  function list_build_row($item, &$form_state, $operations) {
    // Set up sorting
    $name = $item->{$this->plugin['export']['key']};
    $schema = ctools_export_get_schema($this->plugin['schema']);
    $layer = geoserver_get_layer_object($item);

    if (!$layer) return;

    switch ($layer->get_status()) {
      case (GEOSERVER_STATUS_FOUND_SAME):
        $status = t('Same');
        break;

      case (GEOSERVER_STATUS_FOUND_DIFF):
        $status = t('Different');
        break;

      case (GEOSERVER_STATUS_NOT_FOUND):
        $status = t('Unavailable');
        break;
    }

    // Note: $item->{$schema['export']['export type string']} should have already been set up by export.inc so
    // we can use it safely.
    switch ($form_state['values']['order']) {
      case 'disabled':
        $this->sorts[$name] = empty($item->disabled) . $name;
        break;
      case 'title':
        $this->sorts[$name] = $item->{$this->plugin['export']['admin_title']};
        break;
      case 'name':
        $this->sorts[$name] = $name;
        break;
      case 'geoserver':
        $this->sorts[$name] = $status;
        break;
      case 'storage':
        $this->sorts[$name] = $item->{$schema['export']['export type string']} . $name;
        break;
    }

    $this->rows[$name]['data'] = array();
    $this->rows[$name]['class'] = !empty($item->disabled) ? array('ctools-export-ui-disabled') : array('ctools-export-ui-enabled');

    // If we have an admin title, make it the first row.
    if (!empty($this->plugin['export']['admin_title'])) {
      $this->rows[$name]['data'][] = array('data' => check_plain($item->{$this->plugin['export']['admin_title']}), 'class' => array('ctools-export-ui-title'));
    }

    $this->rows[$name]['data'][] = array('data' => $item->title, 'class' => array('ctools-export-ui-title'));
    $this->rows[$name]['data'][] = array('data' => $item->description, 'class' => array('ctools-export-ui-description'));
    $this->rows[$name]['data'][] = array('data' => check_plain($item->{$schema['export']['export type string']}), 'class' => array('ctools-export-ui-storage'));
    $this->rows[$name]['data'][] = array('data' => $status, 'class' => array('ctools-export-ui-geoserver'));

    $ops = theme('links__ctools_dropbutton', array('links' => $operations, 'attributes' => array('class' => array('links', 'inline'))));

    $this->rows[$name]['data'][] = array('data' => $ops, 'class' => array('ctools-export-ui-operations'));

    // Add an automatic mouseover of the description if one exists.
    if (!empty($this->plugin['export']['admin_description'])) {
      $this->rows[$name]['title'] = $item->{$this->plugin['export']['admin_description']};
    }
  }


  /**
   * Provide the table header.
   *
   * If you've added columns via list_build_row() but are still using a
   * table, override this method to set up the table header.
   */
  function list_table_header() {
    $header = array();

    if (!empty($this->plugin['export']['admin_title'])) {
      $header[] = array('data' => t('Name'), 'class' => array('ctools-export-ui-name'));
    }
    $header[] = array('data' => t('Title'), 'class' => array('ctools-export-ui-title'));
    $header[] = array('data' => t('Description'), 'class' => array('ctools-export-ui-description'));
    $header[] = array('data' => t('Storage'), 'class' => array('ctools-export-ui-storage'));
    $header[] = array('data' => t('Geoserver'), 'class' => array('ctools-export-ui-geoserver'));
    $header[] = array('data' => t('Operations'), 'class' => array('ctools-export-ui-operations'));

    return $header;
  }

  /**
   * hook_menu() entry point.
   *
   * Child implementations that need to add or modify menu items should
   * probably call parent::hook_menu($items) and then modify as needed.
   */
  function hook_menu(&$items) {
    parent::hook_menu($items);

    $items['admin/structure/geoserver/layers']['type'] = MENU_LOCAL_TASK;
  }

  /**
   * Provide a list of sort options.
   *
   * Override this if you wish to provide more or change how these work.
   * The actual handling of the sorting will happen in build_row().
   */
  function list_sort_options() {
    if (!empty($this->plugin['export']['admin_title'])) {
      $options = array(
        'disabled' => t('Enabled, title'),
        $this->plugin['export']['admin_title'] => t('Title'),
      );
    }
    else {
      $options = array(
        'disabled' => t('Enabled, title'),
      );
    }

    $options += array(
      'storage' => t('Storage'),
    );

    return $options;
  }

  /**
   * Prepare the tag values before they are added to the database.
   */
  function edit_form_submit(&$form, &$form_state) {
    $form_state['values']['data'] = $form_state['values'][$form_state['values']['layer_type']];
    parent::edit_form_submit($form, $form_state);

    $layer = geoserver_get_layer_object($form_state['item']);
    $layer->save();

    if ($layer->get_status() == GEOSERVER_STATUS_NOT_FOUND) {

      try {
        $resource = $layer->to_resource();
        $result = $resource->create();

      } catch (geoserver_resource_exception $exc) {
        drupal_set_message(t('Error when attempting to create %layer: %message',
            array('%layer' => $layer->title, '%message' => $exc->getMessage())), 'error');
      }

    } else {

      try {
        $resource = $layer->to_resource();
        $resource->update();

      } catch (geoserver_resource_exception $exc) {
        drupal_set_message(t('Error when attempting to update %layer: %message',
            array('%layer' => $layer->title, '%message' => $exc->getMessage())), 'error');
      }
    }
  }

  function delete_form_submit(&$form_state) {

    $layer = geoserver_get_layer_object($form_state['item']);

    try {
      $resource = $layer->to_resource();
      $resource->delete();
      parent::delete_form_submit($form_state);

    } catch (geoserver_resource_exception $exc) {

      drupal_set_message(t('Error when attempting to delete %layer: %message',
          array('%layer' => $layer->title, '%message' => $exc->getMessage())), 'error');
    }
  }
}

/**
 * Submit handler for ctools_export_ui_edit_item_form.
 */
function geoserver_layers_ui_type_form(&$form, &$form_state) {

  $type = $form_state['values']['layer_type'];
  $layer = new $type;
  $layer->name = $form_state['values']['name'];
  $layer->load_source($form_state['values'][$type]['source']);

  return $layer->options_form($form['layer'][$type]['options']);
}