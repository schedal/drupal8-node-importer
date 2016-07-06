<?php
/**
 * @file
 * Contains \Drupal\import_eight\Form\ImportEightForm.
 */

namespace Drupal\import_eight\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Implements custom drupal 8 node import form.
 */
class ImportEightForm extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'import_eight_import_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form['info'] = array(
      '#markup' => $this->t('Start running the custom Drupal 8 import batch.'),
    );

    $form['actions']['#type'] = 'actions';
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => $this->t('Start Batch'),
      '#button_type' => 'primary',
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    $batch = array(
      'title' => t('Processing Drupal 8 Import'),
      'operations' => array(
        array('import_eight_import_nodes', array()),
      ),
      'finished' => 'import_eight_batch_finished',
      'file' => drupal_get_path('module', 'import_eight') . '/import_eight.batch.inc',
    );

    batch_set($batch);
  }

}
