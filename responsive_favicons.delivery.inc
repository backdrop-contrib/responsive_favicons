<?php
/**
 * @file
 *
 * Responsible for sending icons for common paths automatically requested by
 * certain browsers (even though the HTML does not mention them).
 */

/**
 * Creates a file object for the requested icon path.
 *
 * @param string $file_name
 *   The icon or other asset filename.
 * @return array
 *   An array containing a File object.
 */
function responsive_favicons_get_file($file_name) {
  $uri = 'public://' . config_get('responsive_favicons.settings', 'responsive_favicons_path') . '/' . $file_name;

  $file = new File();
  $file->uri = $uri;
  $file->filemime = file_get_mimetype($uri);
  $file->filesize = @filesize($uri);

  return array($file);
}

/**
 * Attempts to send the raw file back in the response.
 *
 * @param array $file
 *   An array containing a File object that represents the icon or other asset.
 *   This is because the functions that look for any alters expect an array,
 *   not an Object.
 * @see http://cgit.drupalcode.org/favicon/tree/src/DrupalFavicon.php#n145
 */
function responsive_favicons_deliver_file(array $file) {
  // Get the File object from the array.
  $file = $file[0];

  // Handle if the file does not exist in the favicon list of files.
  if (is_int($file)) {
    backdrop_deliver_html_page($file);
    return;
  }
  elseif (!is_object($file) || !is_file($file->uri) || !is_readable($file->uri)) {
    backdrop_deliver_html_page(MENU_NOT_FOUND);
    return;
  }

  // If it does exist, serve that file at the request location.
  $headers = array(
    'Content-Type' => mime_header_encode($file->filemime),
    'Content-Disposition' => 'inline',
    'Content-Length' => $file->filesize,
  );

  // Let other modules know the file is being downloaded.
  module_invoke_all('file_transfer', $file->uri, $headers);

  foreach ($headers as $name => $value) {
    backdrop_add_http_header($name, $value);
  }

  $fd = fopen($file->uri, 'rb');
  if ($fd !== FALSE) {
    while (!feof($fd)) {
      print fread($fd, BACKDROP_KILOBYTE);
    }
    fclose($fd);
  }
  else {
    watchdog('responsive_favicons', 'Unable to open @uri for reading.', array('@uri' => $file->uri));
    backdrop_deliver_html_page(MENU_NOT_FOUND);
    return;
  }

  backdrop_page_footer();
}
