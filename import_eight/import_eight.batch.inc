<?php
use \Drupal\node\Entity\Node;
use \Drupal\file\Entity\File;

/**
 * Save nodes.
 */
function import_eight_import_nodes(&$context) {
  // Use the $context['sandbox'] at your convenience to store the
  // information needed to track progression between successive calls.

  if (empty($context['sandbox'])) {
    $context['sandbox'] = array();
    $context['sandbox']['progress'] = 0;

    // load the csv
    $rows = array();
    if (($handle = fopen(drupal_get_path('module', 'import_eight') . "/world5.csv", "r")) !== FALSE) {
      while (($data = fgetcsv($handle)) !== FALSE) {
        $rows[] = $data;
      }
      fclose($handle);
    }
    // columns keys
    $keys = array_shift($rows);

    // save data to context
    $context['sandbox']['keys'] = $keys;
    $context['sandbox']['rows'] = $rows;
    $context['sandbox']['max'] = count($rows);
  }

  // save 5 nodes per loop
  for ($i = 0; $i < 5; $i++) {
    $data = $context['sandbox']['rows'][$context['sandbox']['progress']];
    $keys = $context['sandbox']['keys'];

    // create base node
    $type = array_search('type', $keys);
    $title = array_search('title', $keys);
    $node = Node::create([
      'type' => $data[$type],
      'title' => $data[$title],
      'langcode' => 'en',
      'uid' => 1,
    ]);

    // fields for each type
    switch ($data[array_search('type', $keys)]) {
      case 'front_slide':
        $body = array_search('body', $keys);
        $node->body->setValue([
          'value' => $data[$body],
          'format' => 'full_html',
        ]);
        $uri = array_search("field_link['0']['url']", $keys);
        $l_title = array_search("field_link['0']['title']", $keys);
        $node->field_link->setValue([
          'uri' => $data[$uri],
          'title' => $data[$l_title],
        ]);
        // Create file object from a copied file.
        $filename = array_search("field_image['0']['filename']", $keys);
        $url = 'http://test.world5.org/files/front_slide/' . $data[$filename];
        $date = date('Y-m');
        if ($file = system_retrieve_file($url, "public://$date/" . $data[$filename], TRUE)) {
          $node->field_main_image->setValue([
            'target_id' => $file->id(),
            'alt' => $data[$filename],
          ]);
        }
        break;
      case 'page':
      case 'blog':
        $body = array_search('body', $keys);
        $node->body->setValue([
          'value' => $data[$body],
          'format' => 'full_html',
        ]);
        // check all the term values
        $term_keys = array();
        foreach ($keys as $id => $key) {
          if (stripos($key, 'taxonomy') === 0) {
            $term_keys[] = $id;
          }
        }
        foreach ($term_keys as $key) {
          $term = import_eight_term_map($data[$key]);
          if (!empty($term)) {
            $node->field_taxonomy[] = array('target_id' => $term->id());
          }
        }
        // Create file object from a locally copied file.
        $filename = array_search("field_main_image['0']['filename']", $keys);
        $url = 'http://test.world5.org/files/' . $data[$filename];
        $date = date('Y-m');
        if ($file = system_retrieve_file($url, "public://$date/" . $data[$filename], TRUE)) {
          $node->field_main_image->setValue([
            'target_id' => $file->id(),
            'alt' => $data[$filename],
          ]);
        }
        // video url
        $video = array_search("field_video['0']['embed']", $keys);
        if (!empty($data[$video]) && $data[$video] != 'NULL') {
          $node->field_video->setValue([
            'value' => $data[$video],
          ]);
        }
        break;
    }

    // save new node
    $node->save();
    // save path
    $path = array_search('path', $keys);
    \Drupal::service('path.alias_storage')->save('/node/' . $node->id(), '/' . $data[$path], 'en');

    $context['sandbox']['progress']++;
    $context['message'] = t('Now processing node :lid of :total.', array(':lid' => $context['sandbox']['progress'], ':total' => $context['sandbox']['max']));

    // Inform the batch engine that we are not finished,
    // and provide an estimation of the completion level we reached.
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
    if ($context['finished'] >= 1) {
      break;
    }
  }
}

/**
 * Batch finished.
 */
function import_eight_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message(t('D8 Import is finished.'));
  }
  else {
    // An error occurred.
    drupal_set_message(t('An error occurred with D8 Import.'), 'error');
  }
}

/**
 * Map TID to term name for search.
 */
function import_eight_term_map($tid) {
  $map = array(
    28 => 'Activism',
    30 => 'Awareness',
    16 => 'Clean Air',
    5 => 'Commerce',
    22 => 'Community',
    10 => 'Constitution',
    32 => 'Culture',
    29 => 'Democracy',
    2 => 'Ecology',
    6 => 'Energy',
    31 => 'Food',
    4 => 'Government',
    34 => 'Health',
    14 => 'Holistic',
    38 => 'Human Rights',
    41 => 'Humor',
    11 => 'Localism',
    12 => 'Organic',
    7 => 'Peace',
    35 => 'Permaculture',
    15 => 'Politics',
    18 => 'Recycle',
    13 => 'Sustainable',
    1 => 'Transformation',
    17 => 'Water',
  );

  if (empty($map[$tid])) {
    return NULL;
  }

  $terms = taxonomy_term_load_multiple_by_name($map[$tid]);

  return reset($terms);
}

// Array
// (
//     [0] => nid
//     [1] => type
//     [2] => language
//     [3] => uid
//     [4] => status
//     [5] => created
//     [6] => changed
//     [7] => comment
//     [8] => promote
//     [9] => moderate
//     [10] => sticky
//     [11] => tnid
//     [12] => translate
//     [13] => vid
//     [14] => revision_uid
//     [15] => title
//     [16] => body
//     [17] => teaser
//     [18] => revision_timestamp
//     [19] => format
//     [20] => name
//     [21] => picture
//     [22] => data
//     [23] => path
//     [24] => field_main_image['0']['fid']
//     [25] => field_main_image['0']['uid']
//     [26] => field_main_image['0']['filename']
//     [27] => field_main_image['0']['filepath']
//     [28] => field_main_image['0']['filemime']
//     [29] => field_main_image['0']['filesize']
//     [30] => field_main_image['0']['status']
//     [31] => field_main_image['0']['timestamp']
//     [32] => field_main_image['0']['list']
//     [33] => field_main_image['0']['data']['alt']
//     [34] => field_main_image['0']['data']['title']
//     [35] => field_main_image['0']['node_export_file_url']
//     [36] => field_video['0']['embed']
//     [37] => field_video['0']['value']
//     [38] => field_video['0']['provider']
//     [39] => field_video['0']['data']
//     [40] => field_video['0']['version']
//     [41] => field_video['0']['duration']
//     [42] => field_audio['0']
//     [43] => last_comment_timestamp
//     [44] => last_comment_name
//     [45] => comment_count
//     [46] => taxonomy['1']['28']
//     [47] => taxonomy['1']['22']
//     [48] => taxonomy['1']['32']
//     [49] => taxonomy['1']['29']
//     [50] => taxonomy['1']['2']
//     [51] => taxonomy['1']['6']
//     [52] => taxonomy['1']['31']
//     [53] => taxonomy['1']['4']
//     [54] => taxonomy['1']['34']
//     [55] => taxonomy['1']['38']
//     [56] => taxonomy['1']['11']
//     [57] => taxonomy['1']['12']
//     [58] => taxonomy['1']['35']
//     [59] => taxonomy['1']['1']
//     [60] => nodewords['abstract']['value']
//     [61] => nodewords['canonical']['value']
//     [62] => nodewords['canonical']['hide']
//     [63] => nodewords['copyright']['value']
//     [64] => nodewords['dcterms.contributor']['value']
//     [65] => nodewords['dcterms.creator']['value']
//     [66] => nodewords['dcterms.creator']['auto']
//     [67] => nodewords['dcterms.date']['value']['month']
//     [68] => nodewords['dcterms.date']['value']['day']
//     [69] => nodewords['dcterms.date']['value']['year']
//     [70] => nodewords['dcterms.description']['value']
//     [71] => nodewords['dcterms.title']['value']
//     [72] => nodewords['description']['value']
//     [73] => nodewords['geo.placename']['value']
//     [74] => nodewords['geo.region']['value']
//     [75] => nodewords['keywords']['value']
//     [76] => nodewords['location']['latitude']
//     [77] => nodewords['location']['longitude']
//     [78] => nodewords['original-source']['value']
//     [79] => nodewords['page_title']['value']
//     [80] => nodewords['page_title']['append']
//     [81] => nodewords['page_title']['divider']
//     [82] => nodewords['revisit-after']['value']
//     [83] => nodewords['robots']['value']['index']
//     [84] => nodewords['robots']['value']['noindex']
//     [85] => nodewords['robots']['value']['follow']
//     [86] => nodewords['robots']['value']['nofollow']
//     [87] => nodewords['robots']['value']['noarchive']
//     [88] => nodewords['robots']['value']['noodp']
//     [89] => nodewords['robots']['value']['nosnippet']
//     [90] => nodewords['robots']['value']['noydir']
//     [91] => nodewords['robots']['use_default']
//     [92] => nodewords['shorturl']['value']
//     [93] => nodewords['standout']['value']
//     [94] => nodewords['syndication-source']['value']
//     [95] => uuid
//     [96] => is_new
//     [97] => timestamp
//     [98] => menu['link_title']
//     [99] => menu['mlid']
//     [100] => menu['plid']
//     [101] => menu['menu_name']
//     [102] => menu['weight']
//     [103] => menu['module']
//     [104] => menu['expanded']
//     [105] => menu['hidden']
//     [106] => menu['has_children']
//     [107] => menu['customized']
//     [108] => menu['parent_depth_limit']
//     [109] => field_link['0']['url']
//     [110] => field_link['0']['title']
//     [111] => field_link['0']['attributes']
//     [112] => field_image['0']['fid']
//     [113] => field_image['0']['uid']
//     [114] => field_image['0']['filename']
//     [115] => field_image['0']['filepath']
//     [116] => field_image['0']['filemime']
//     [117] => field_image['0']['filesize']
//     [118] => field_image['0']['status']
//     [119] => field_image['0']['timestamp']
//     [120] => field_image['0']['list']
//     [121] => field_image['0']['data']['alt']
//     [122] => field_image['0']['data']['title']
//     [123] => tags
//     [124] => field_main_image['0']
//     [125] => taxonomy['1']['30']
//     [126] => taxonomy['1']['14']
//     [127] => taxonomy['1']['7']
//     [128] => taxonomy['1']['10']
//     [129] => taxonomy['1']['18']
//     [130] => taxonomy['1']['13']
//     [131] => taxonomy['1']['5']
// )
