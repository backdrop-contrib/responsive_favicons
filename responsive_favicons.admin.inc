<?php
/**
 * @file
 * Admin page callbacks for the responsive_favicons module.
 */

/**
 * Menu callback; Provide the administration overview page.
 */
function responsive_favicons_config_page() {
  $config = config('responsive_favicons.settings');
  $form['responsive_favicons_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Path to responsive favicon files'),
    '#description' => t('A local file system path where favicon files will be stored. This directory must exist and be writable by Backdrop.'),
    '#field_prefix' => file_create_url('public://'),
    '#default_value' => $config->get('responsive_favicons_path'),
  );
  $form['responsive_favicons_tags'] = array(
    '#type' => 'textarea',
    '#title' => t('Favicon tags'),
    '#description' => t('Paste the code provided by <a href="@real-favicon-generator" target="_blank">RealFaviconGenerator</a>. Make sure each link is on a separate line. It is fine to paste links with paths like "/apple-touch-icon.png" as these will be converted to the correct paths automatically.', array(
      '@real-favicon-generator' => 'https://realfavicongenerator.net/',
    )),
    '#default_value' => implode(PHP_EOL, $config->get('responsive_favicons_tags')),
    '#rows' => 16,
  );
  $form['responsive_favicons_upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload the zip file you downloaded from RealFaviconGenerator.'),
    '#description' => t('For example: %filename from your local computer. This only needs to be done once.', array('%filename' => 'favicons.zip')),
  );
  $form['responsive_favicons_remove_default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove default favicon from Backdrop'),
    '#description' => t('It is recommended to remove default favicon as it can cause issues'),
    '#default_value' => $config->get('responsive_favicons_remove_default'),
  );

  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['#submit'][] = 'responsive_favicons_config_page_submit';
  return $form;
}

/**
 * Implements additional submit logic for responsive_favicons_settings_form().
 */
function responsive_favicons_config_page_submit($form, &$form_state) {
    $config = config('responsive_favicons.settings');
  // We want to save tags as an array.
  if (isset($form_state['values']['responsive_favicons_tags'])) {
    $tags = explode(PHP_EOL, $form_state['values']['responsive_favicons_tags']);
    $tags = array_filter($tags);
    foreach ($tags as $pos => $tag) {
      $tags[$pos] = trim($tag);
    }
    $config->set('responsive_favicons_tags', $tags);
  }

  // Remove trailing slash on responsive_favicons_path.
  $form_state['values']['responsive_favicons_path'] = rtrim($form_state['values']['responsive_favicons_path'], '/');

  // Attempt the upload and extraction of the zip file. This code is largely
  // based on the code in Backdrop core.
  //
  // @see update_manager_install_form_submit().
  if ($_FILES['files']['name']['responsive_favicons_upload']) {
    $validators = array('file_validate_extensions' => array(archiver_get_extensions()));
    $field = 'responsive_favicons_upload';
    if (!($finfo = file_save_upload($field, $validators, NULL, FILE_EXISTS_REPLACE))) {
      // Failed to upload the file. file_save_upload() calls form_set_error() on
      // failure.
      return;
    }
    $local_cache = $finfo->uri;

    $directory = _responsive_favicons_extract_directory();
    try {
      $archive = _responsive_favicons_archive_extract($local_cache, $directory);
    }
    catch (Exception $e) {
      form_set_error($field, $e->getMessage());
      return;
    }

    $files = $archive->listContents();
    if (!$files) {
      form_set_error($field, t('Provided archive contains no files.'));
      return;
    }

    // Create the destination directory.
    $destination = 'public://' . $config->get('responsive_favicons_path');
    file_prepare_directory($destination, FILE_CREATE_DIRECTORY);

    // Copy the files to the correct location.
    $success_count = 0;
    foreach ($files as $file) {
      $success = file_unmanaged_copy($directory . '/' . $file, $destination, FILE_EXISTS_REPLACE);
      $uri = $destination . '/' . $file;
      if ($success) {
        $success_count++;

        // Rewrite the paths of the JSON files.
        if (preg_match('/\.json$/', $file)) {
          $file_contents = file_get_contents(backdrop_realpath($uri));
          $find = preg_quote('"\/android-chrome', '/');
          $replace = '"' . str_replace('/', '\/', _responsive_favicons_normalise_path('/android-chrome'));
          $file_contents = preg_replace('/' . $find . '/', $replace, $file_contents);
          file_unmanaged_save_data($file_contents, $uri, FILE_EXISTS_REPLACE);
          continue;
        }
        // Rewrite the paths of the XML files.
        if (preg_match('/\.xml$/', $file)) {
          $file_contents = file_get_contents(backdrop_realpath($uri));
          $find = preg_quote('"/mstile', '/');
          $replace = '"' . _responsive_favicons_normalise_path('/mstile');
          $file_contents = preg_replace('/' . $find . '/', $replace, $file_contents);
          file_unmanaged_save_data($file_contents, $uri, FILE_EXISTS_REPLACE);
          continue;
        }
        // Rewrite the paths of site.webmanifest files.
        if (preg_match('/site\.webmanifest$/', $file)) {
          $file_contents = file_get_contents(backdrop_realpath($uri));
          $site_manifest = backdrop_json_decode($file_contents);
          foreach ($site_manifest['icons'] as $index => $icon) {
            $icon_src = _responsive_favicons_normalise_path($icon['src']);
            $site_manifest['icons'][$index]['src'] = $icon_src;
          }
          $file_contents = backdrop_json_encode($site_manifest, TRUE);
          file_unmanaged_save_data($file_contents, $uri, FILE_EXISTS_REPLACE);
        }
      }
    }

    if ($success_count > 0) {
      backdrop_set_message(format_plural($success_count, 'Uploaded 1 favicon successfully.', 'Uploaded @count favicons successfully.'));
    }
  }

  $config->set('responsive_favicons_path', $form_state['values']['responsive_favicons_path']);
  $config->set('responsive_favicons_remove_default', $form_state['values']['responsive_favicons_remove_default']);
  $config->save();
  backdrop_set_message(t('The configuration options have been saved.'));
}

/**
 * Returns a short unique identifier for this Backdrop installation.
 *
 * @return string
 *   An eight character string uniquely identifying this Backdrop installation.
 */
function _responsive_favicons_unique_identifier() {
  $id = &backdrop_static(__FUNCTION__, '');
  if (empty($id)) {
    $id = substr(hash('sha256', backdrop_get_hash_salt()), 0, 8);
  }
  return $id;
}

/**
 * Returns the directory where update archive files should be extracted.
 *
 * @param bool $create
 *   (optional) Whether to attempt to create the directory if it does not
 *   already exist. Defaults to TRUE.
 *
 * @return string
 *   The full path to the temporary directory where update file archives should
 *   be extracted.
 */
function _responsive_favicons_extract_directory($create = TRUE) {
  $directory = &backdrop_static(__FUNCTION__, '');
  if (empty($directory)) {
    $directory = 'temporary://responsive-favicons.' . _responsive_favicons_unique_identifier();
    if ($create && !file_exists($directory)) {
      mkdir($directory);
    }
  }
  return $directory;
}

/**
 * Unpacks a downloaded archive file.
 *
 * @param string $file
 *   The filename of the archive you wish to extract.
 * @param string $directory
 *   The directory you wish to extract the archive into.
 *
 * @return Archiver
 *   The Archiver object used to extract the archive.
 *
 * @throws Exception
 */
function _responsive_favicons_archive_extract($file, $directory) {
  $archiver = archiver_get_archiver($file);
  if (!$archiver) {
    throw new Exception(t('Cannot extract %file, not a valid archive.', array ('%file' => $file)));
  }

  if (file_exists($directory)) {
    file_unmanaged_delete_recursive($directory);
  }

  $archiver->extract($directory);
  return $archiver;
}
