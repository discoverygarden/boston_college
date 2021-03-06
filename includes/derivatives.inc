<?php
/**
 * @file
 * Defines functions used when constructing derivatives.
 */

/**
 * Updates the label of an object from the DC title field.
 *
 * Force isn't checked here as this action will always occur depending on
 * configuration.
 *
 * @param AbstractObject $object
 *   An AbstractObject representing an object within Fedora.
 * @param bool $force
 *   Whether the derivative is being forcefully created or not.
 *
 * @return array
 *   An array describing the outcome of the label updation.
 *
 * @see hook_islandora_derivative()
 */
function boston_college_update_object_label_from_mods_derivative(AbstractObject $object, $force = FALSE) {
  $mods_doc = new DOMDocument();
  $mods_doc->loadXML($object['MODS']->content);
  $xpath = new DOMXPath($mods_doc);
  $xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
  $title_infos = $xpath->query('/mods:mods/mods:titleInfo');
  if ($title_infos->length === 0) {
    return array(
      'success' => FALSE,
      'messages' => array(
        array(
          'message' => t('Failed to find mods:titleInfo to update the label of @pid.'),
          'message_sub' => array(
            '@pid' => $object->id,
          ),
          'type' => 'dsm',
        ),
      ),
    );
  }
  $title_info = $title_infos->item(0);
  $title = $xpath->evaluate('string(mods:title)', $title_info);
  $non_sort = $xpath->evaluate('string(mods:nonSort)', $title_info);
  $label = "$non_sort$title";
  if ($label && $object->label != $label) {
    $object->label = $label;
  }
  return array(
    'success' => TRUE,
    'messages' => array(
      array(
        'message' => t('Updated the label for @pid.'),
        'message_sub' => array(
          '@pid' => $object->id,
        ),
        'type' => 'dsm',
      ),
    ),
  );
}

/**
 * Creates a redirect stream for streaming a video.
 *
 * @see hook_islandora_derivative()
 */
function boston_college_create_stream(AbstractObject $object, $force = FALSE) {
  if (isset($object['MODS'])) {
    $mods_xml = new SimpleXMLElement($object['MODS']->content);
    $streams = $mods_xml->xpath('/mods:mods/mods:location/mods:url');
    $stream = (string) reset($streams);
    // Bail out if stream is not valid URL (Fedora goes 500).
    if (!filter_var($stream, FILTER_VALIDATE_URL)) {
      return array(
        'success' => FALSE,
        'messages' => array(
          array(
            'message' => t('Failed to derive STREAM for @pid, URL "!url" invalid.'),
            'message_sub' => array(
              '@pid' => $object->id,
              '!url' => $stream,
            ),
            'type' => 'dsm',
          ),
        ),
      );
    }

    if (isset($object['STREAM'])) {
      $datastream = $object['STREAM'];
    }
    else {
      $datastream = $object->constructDatastream('STREAM', 'R');
      $datastream->mimetype = 'text/html';
      $datastream->label = 'Video player HTML page.';
    }

    $datastream->url = $stream;
    // Doesn't do anything if ingested already.
    $object->ingestDatastream($datastream);

    return array(
      'success' => TRUE,
      'messages' => array(
        array(
          'message' => t('Derived STREAM for @pid.'),
          'message_sub' => array(
            '@pid' => $object->id,
          ),
          'type' => 'dsm',
        ),
      ),
    );
  }
  return array();
}
