<?php
/**
 * @file
 */

class geoserver_styles_ui extends ctools_export_ui {

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

    switch (geoserver_style_status($item)) {
      case (GEOSERVER_STATUS_FOUND_SAME):
        $status = t('Same');
        break;

      case (GEOSERVER_STATUS_FOUND_DIFF):
        $status = t('Different');
        $operations['update'] = array(
          'title' => 'Update',
          'href' => "admin/structure/geoserver/styles/list/{$item->name}/update",
        );
        break;

      case (GEOSERVER_STATUS_NOT_FOUND):
        $status = t('Unavailable');
        $operations['create'] = array(
          'title' => 'Create',
          'href' => "admin/structure/geoserver/styles/list/{$item->name}/create",
        );
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

    $items['admin/structure/geoserver/styles']['type'] = MENU_LOCAL_TASK;

    $items['admin/structure/geoserver/styles/list/%ctools_export_ui/update'] = array(
      'title' => 'Update style',      
      'description' => 'Update GeoServer style.',
      'page callback' => 'ctools_export_ui_switcher_page',
      'page arguments' => array('geoserver_styles_ctools_export_ui', 'update', 5),
      'load arguments' =>  array('geoserver_styles_ctools_export_ui'),
      'access arguments' => array('administer geoserver'),
      'type' => MENU_NORMAL_ITEM,
    );

    $items['admin/structure/geoserver/styles/list/%ctools_export_ui/create'] = array(
      'title' => 'Create style',
      'description' => 'Create GeoServer style.',
      'page callback' => 'ctools_export_ui_switcher_page',
      'page arguments' => array('geoserver_styles_ctools_export_ui', 'create', 5),
      'load arguments' =>  array('geoserver_styles_ctools_export_ui'),
      'access arguments' => array('administer geoserver'),
      'type' => MENU_NORMAL_ITEM,
    );
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

  function delete_form_submit(&$form_state) {

    $item = $form_state['item'];

    try {
      geoserver_style_delete($item);
    }
    catch (Exception $exc) {
      drupal_set_message(t('Error when attempting to delete %style: %message',
          array('%style' => $item->title, '%message' => $exc->getMessage())), 'error');
    }

    parent::delete_form_submit($form_state);
  }

  function update_page($js, $input, $item, $step = NULL) {

    $form_state = array('object' => &$this, 'item' => $item);
    return drupal_build_form('geoserver_styles_ui_update_form', $form_state);
  }

  function update_form(&$form, &$form_state) {

    $style = $form_state['item'];
    $form = array('style' => array('#type' => 'value', '#value' => $style));
    $message = t('Are you sure you want to update %style?', array('%style' => $style->title));
    $form = confirm_form($form, $message, 'admin/structure/geoserver/styles');
  }

  /**
   * Submit callback for style update form.
   */
  function update_form_submit($form, &$form_state) {

    $style = $form_state['values']['style'];
    try {
      geoserver_style_update($style);
      drupal_set_message(t('%style was updated.', array('%style' => $style->title)));
    } catch (geoserver_resource_exception $exc) {
      drupal_set_message(t('Error when attempting to update %style: %message',
          array('%style' => $style->title, '%message' => $exc->getMessage())), 'error');
    }

    // Redirect.
    $form_state['redirect'] = 'admin/structure/geoserver/styles';
  }

  function create_page($js, $input, $item, $step = NULL) {

    $form_state = array('object' => &$this, 'item' => $item);
    return drupal_build_form('geoserver_styles_ui_create_form', $form_state);
  }

  /**
   * Style create form.
   */
  function create_form(&$form, &$form_state) {

    $style = $form_state['item'];

    // Create create form.
    $form = array('style' => array('#type' => 'value', '#value' => $style));
    $message = t('Are you sure you want to create %style?', array('%style' => $style->title));
    $form = confirm_form($form, $message, 'admin/structure/geoserver/styles');
  }

  /**
   * Submit callback for style create form.
   */
  function create_form_submit($form, &$form_state) {

    $style = $form_state['values']['style'];

    try {
      geoserver_style_create($style);
      drupal_set_message(t('%style was created.', array('%style' => $style->title)));
    }
    catch (Exception $exc) {
      drupal_set_message(t('Error when attempting to create %style: %message',
         array('%style' => $style->title, '%message' => $exc->getMessage())), 'error');
    }

    $form_state['redirect'] = 'admin/structure/geoserver/styles';
  }
}

/**
 * Form callback to edit an exportable item.
 *
 * This simply loads the object defined in the plugin and hands it off.
 */
function geoserver_styles_ui_update_form($form, &$form_state) {
  $form_state['object']->update_form($form, $form_state);
  return $form;
}

/**
 * Submit handler for ctools_export_ui_edit_item_form.
 */
function geoserver_styles_ui_update_form_submit(&$form, &$form_state) {
  $form_state['object']->update_form_submit($form, $form_state);
}

/**
 * Form callback to edit an exportable item.
 *
 * This simply loads the object defined in the plugin and hands it off.
 */
function geoserver_styles_ui_create_form($form, &$form_state) {
  $form_state['object']->create_form($form, $form_state);
  return $form;
}

/**
 * Submit handler for ctools_export_ui_edit_item_form.
 */
function geoserver_styles_ui_create_form_submit(&$form, &$form_state) {
  $form_state['object']->create_form_submit($form, $form_state);
}
