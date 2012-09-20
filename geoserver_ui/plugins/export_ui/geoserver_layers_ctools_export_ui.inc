<?php
/**
 * @file
 * Define the Export UI plugin for GeoServer Layers.
 */

$plugin = array(
  'schema' => 'geoserver_layers',
  'access' => 'administer geoserver',
  'menu' => array(
    'menu prefix' => 'admin/structure/geoserver',
    'menu item' => 'layers',
    'menu type' => MENU_LOCAL_TASK,
    'menu title' => 'Layers',
    'menu description' => 'Administer GeoServer Layers.',
  ),

  'handler' => array(
    'class' => 'geoserver_layers_ui',
  ),

  'export' => array(
    'admin_title' => 'name',
    'admin_description' => 'description',
  ),

  'title singular' => t('layer'),
  'title plural' => t('layers'),
  'title singular proper' => t('GeoServer Layer'),
  'title plural proper' => t('GeoServer Layers'),

  'strings' => array(
    'confirmation' => array(
      'add' => array(
        'success' => t('Layer added.'),
        'fail' => t('Failed to completely add layer. See the log for details.'),
      ),
      'edit' => array(
        'success' => t('Layer updated.'),
        'fail' => t('Failed to completely update layer. See the log for details.'),
      ),
      'delete' => array(
        'success' => t('Layer was deleted.'),
        'fail' => t('Failed to completely delete layer. See the log for details.'),
      )
    )
  ),

  'form' => array(
    'settings' => 'geoserver_layers_ctools_export_ui_form_settings',
  ),
);

function geoserver_layers_ctools_export_ui_form_settings(&$form, &$form_state) {

  $layer = geoserver_get_layer_object($form_state['item']);

  if ($form_state['op'] == 'add'||'edit') {
    $form = array();

    $styles = array('');
    foreach (geoserver_styles() as $style) {
      $styles[$style->name] = $style->title;
    }

    $form['layer']['basics'] = array(
      '#tree' => TRUE,
      '#type' => 'fieldset',
      '#title' => t('Layer Basics'),
      '#description' => t('The following settings are for all layers.'),
      '#tree' => FALSE,
    );
    $form['layer']['basics']['title'] = array(
      '#type' => 'textfield',
      '#title' => t('Layer Title'),
      '#required' => TRUE,
      '#size' => 30,
      '#default_value' => isset($layer->title) ? $layer->title : '',
      '#description' => t('The friendly name of your layer, which will appear in the administration interface.'),
    );
    $form['layer']['basics']['name'] = array(
      '#type' => 'machine_name',
      '#maxlength' => 32,
      '#machine_name' => array(
        'exists' => 'geoserver_layer_load',
        'source' => array('layer', 'basics', 'title'),
      ),
      '#default_value' => isset($layer->name) ? $layer->name : '',
      '#description' => t('This is the unique name of the layer. It must contain only alphanumeric characters and underscores.'),
    );
    $form['layer']['basics']['description'] = array(
      '#type' => 'textfield',
      '#title' => t('Layer Description'),
      '#default_value' => isset($layer->description) ? $layer->description : '',
      '#description' => t('The description of your layer, which will appear in the administration interface.'),
    );

    $layers_options = array();
    $layer_types = geoserver_layer_types();
    $layer_type_options = array('' => t('Select the layer type'));

    foreach ($layer_types as $layer_type) {

      $layer_type_name = $layer_type['name'];

      if (!$layer) {
        $layer = geoserver_layer_type_load($layer_type_name);
      }

      $layers_option = array(
        '#tree' => TRUE,
        '#type' => 'fieldset',
        '#title' => t('Layer Configuration'),
      );

      $layers_option['type'] = array(
        '#type' => 'hidden',
        '#value' => $layer_type_name,
      );

      $layers_option['style'] = array(
        '#type' => 'select',
        '#title' => t('Style'),
        '#options' => $styles,
        '#default_value' => isset($layer->data['style']) ? $layer->data['style'] : '',
        '#description' => t('Please select a default style for this layer.'),
      );

      $layers_option['source'] = array(
        '#type' => 'select',
        '#title' => t('Source'),
        '#options' => call_user_func($layer_type_name . '::get_sources'),
        '#default_value' => isset($layer->data['source']) ? $layer->data['source'] : '',
        '#description' => t('Please select a source for this layer.'),
        '#ajax' => array(
          'callback' => 'geoserver_layers_ui_type_form',
          'wrapper' => $layer_type_name,
        ),
      );

      if (module_exists('openlayers')) {
        $openlayers = isset($layer->data['openlayers']) ? $layer->data['openlayers'] : array();
        $layers_option['openlayers'] = array(
          '#type' => 'fieldset',
          '#title' => t('OpenLayers'),
          '#description' => t('Select how you want to have this layer available in maps of the OpenLayers module. Note that raster data is only available for inclusion in the OpenLayers module if the projection chosen here matches the OpenLayers map in question.'),
        );
        $layers_option['openlayers']['wfs'] = array(
          '#type' => 'checkbox',
          '#title' => t('Provide an OpenLayers WFS layer.'),
          '#default_value' => isset($openlayers['wfs']) ? $openlayers['wfs'] : '',
          '#description' => t('This makes your layer\'s content available as vector data.'),
        );
        $layers_option['openlayers']['gwc'] = array(
          '#type' => 'checkbox',
          '#title' => t('Provide a cached OpenLayers WMS layer.'),
          '#default_value' => isset($openlayers['gwc']) ? $openlayers['gwc'] : '',
          '#description' => t('Creates a layer that transmits its data using several small adjacent images.'),
        );
        // TODO: merge *_srs fields, depends on Drupal 7.13
        // Conditional visibility: http://drupal.org/node/1464758
        $layers_option['openlayers']['gwc_srs'] = array(
          '#type' => 'select',
          '#options' => array(
            'EPSG:4326' => '4326',
            'EPSG:900913' => '900913',
          ),
          '#default_value' => isset($openlayers['gwc_srs']) ? $openlayers['gwc_srs'] : '',
          '#states' => array(
            'visible' => array(
              ':input[name="' . $layer_type_name . '[openlayers][gwc]"]' => array(
                'checked' => TRUE,
              ),
            ),
          ),
          '#description' => t('Choose projection of the generated raster graphics. This should match the projection of any other layers that you want to use in combination with your layer. Select 900913 if you intend to overlay your layer on maps by other providers such as OpenStreetMap, Google, Bing or MapQuest.'),
          '#prefix' => '<div style="padding-left:20px">',
          '#suffix' => '</div>',
        );
        $layers_option['openlayers']['wms'] = array(
          '#type' => 'checkbox',
          '#title' => t('Provide an untiled OpenLayers WMS layer.'),
          '#default_value' => isset($openlayers['wms']) ? $openlayers['wms'] : '',
          '#description' => t('Creates a layer that transmits its data using one image that spans the entire map.'),
        );
        $layers_option['openlayers']['wms_srs'] = array(
          '#type' => 'select',
          '#options' => array(
            'EPSG:4326' => '4326',
            'EPSG:900913' => '900913',
          ),
          '#default_value' => isset($openlayers['wms_srs']) ? $openlayers['wms_srs'] : '',
          '#states' => array(
            'visible' => array(
              ':input[name="' . $layer_type_name . '[openlayers][wms]"]' => array(
                'checked' => TRUE,
              ),
            ),
          ),
          '#description' => t('Choose projection of the generated raster graphics. This should match the projection of any other layers that you want to use in combination with your layer. Select 900913 if you intend to overlay your layer on maps by other providers such as OpenStreetMap, Google, Bing or MapQuest.'),
          '#prefix' => '<div style="padding-left:20px">',
          '#suffix' => '</div>',
        );
      }

      $layers_option['options'] = array(
        '#type' => 'fieldset',
        '#title' => t('Layer Specifics'),
        '#description' => t('The following settings are specific to the layer type.'),
      );

      // Arrays merged in order so that options_form can override defaults.
      //$layers_option['options'] = $layer->options_form() + $layers_option['options'];
      if (method_exists($layer, 'options_form')) {
        $layers_option['options'] += $layer->options_form();
        $layers_option['#states'] = array(
          'visible' => array(
            ':input[name="layer_type"]' => array('value' => $layer_type_name),
          ),
        );
        $layers_options[$layer_type_name] = $layers_option;
      }
      $layer_type_options[$layer_type_name] = $layer_type['title'];
    }

    $form['layer']['layer_type'] = array(
      '#type' => 'select',
      '#title' => t('Layer Type'),
      '#default_value' => isset($layer->data['type']) ? $layer->data['type']: '',
      '#description' => t('Select the type of layer.'),
      '#options' => $layer_type_options,
    );

    $form['layer'] += $layers_options;
    
  }
}


