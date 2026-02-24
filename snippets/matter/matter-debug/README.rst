.. _snippet_matter_debug:
.. _ug_matter_debug_snippet:

Matter debug snippet (matter-debug)
###################################

.. contents::
   :local:
   :depth: 2

Overview
********

The Matter debug snippet allows you to enable additional debug features while using Matter samples.
This snippet is used in the Matter samples to enable additional debug features.

The following features are enabled when using this snippet:

  * UART speed is increased to 1 Mbit/s.
  * Log buffer size is set to high value to allow showing all logs.
  * Deferred mode of logging.
  * Increased verbosity of Matter logs.
  * OpenThread is built from sources.
  * OpenThread shell is enabled.
  * OpenThread logging level is set to INFO.
  * Full shell functionalities.
  * Logging source code location on VerifyOrDie failure that occurs in the Matter stack.

To use the snippet when building a sample, add ``-D<project_name>_SNIPPET=matter-debug`` to the west arguments list.

For example, for the ``nrf52840dk/nrf52840`` board target and the :ref:`matter_lock_sample` sample, use the following command:

.. parsed-literal::
   :class: highlight

   west build -b nrf52840dk/nrf52840 -- -Dlock_SNIPPET=matter-debug

.. note::

   You can increase the UART speed using this snippet only for Nordic Semiconductor's development kits.
   If you want to use the snippet for your custom board, you need to adjust the UART speed manually.
