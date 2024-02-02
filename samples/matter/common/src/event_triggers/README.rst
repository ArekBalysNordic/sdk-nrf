.. _matter_testing_events:

Matter testing events
#####################

.. contents::
   :local:
   :depth: 2

Matter testing events are a set of device's events that can be activated using triggers that have defined the activation codes.
You can use several events only if the specific Kconfig options are set to the specific values.

All event triggers requires the `enable key` that can be defined in the factory data.
The default `enable key` is set to ``00112233445566778899AABBCCDDEEFF``.

Requirements
************

To use the matter testing events you need the :ref:`Chip Tool application <ug_matter_gs_tools_chip>` to the send `test-event-trigger` commands of the general diagnostics cluster.
You need also a device that contains the `generaldiagnostics` cluster in its data model database.
To learn how to set the `generaldiagnostics` cluster see the :ref:`ug_matter_gs_adding_cluster` page.

To activate several triggers you need to have a specific accessory.
For example to use commands for the Door Lock cluster you need to have an accessory that contains that cluster and has set the appropriate product ID.

Available event triggers
************************

The following table lists available triggers and the activation codes:

.. list-table::
    :widths: auto
    :header-rows: 1

   * - Name
     - Requirements
     - Description
     - Activation code
   * - Factory reset
     - None
     - Perform a factory reset of the device.
     - `0xF000000000000000`
   * - Reboot
     - None
     - Reboot the device.
     - `0xF000000000000001`
   * - Diagnostic logs crash
     - The snippet `diagnostic-logs` attached (-S diagnostic-logs) or :kconfig:option:`CONFIG_NCS_SAMPLE_MATTER_DIAGNOSTIC_LOGS` = ``y`` & :kconfig:option:`CONFIG_NCS_SAMPLE_MATTER_DIAGNOSTIC_LOGS_CRASH_LOGS` to ``y`` & `diagnostic logs` cluster
     - Trigger a simple crash that rely on execution of the undefined instruction attempt.
     - `0x1000000000000000`
   * - OTA query
     - :kconfig:option:`CONFIG_CHIP_OTA_REQUESTOR` = ``y``
     - Trigger a OTA firmware update.
     - `0x0100000000000100`
   * - Door lock jammed
     - :kconfig:option:`CONFIG_CHIP_DEVICE_PRODUCT_ID` = ``32774``
     - Simulate the jammed lock state.
       In this state the door lock
     - `0x3277400000000000`

Setting the enable key
**********************

The `enable key` can be set to the specific value only if the :kconfig:option:`CONFIG_CHIP_FACTORY_DATA` Kconfig option is set to ``y``.
Otherwise the default value ``00112233445566778899AABBCCDDEEFF`` will be set.

To set the `enable key` using the build system set :kconfig:option:`CONFIG_CHIP_FACTORY_DATA_BUILD` Kconfig option to ``y`` and the :kconfig:option:`CONFIG_CHIP_DEVICE_ENABLE_KEY` to the hex string with length of 32 Bytes.

If the :kconfig:option:`CONFIG_CHIP_FACTORY_DATA_BUILD` is set to ``n`` you can use the Python script to generate the factory data set with the specific `enable key` value.
To learn how to do it read the :doc:`matter:nrfconnect_factory_data_configuration` guide in the Matter documentation.

Usage
*****

To trigger a specific event on the device, fill the `enable key`, `activation code`, and `node id` fields and run the following command:

.. code-block:: console

   ./chip-tool generaldiagnostics test-event-trigger hex:<enable key> <activation code> <node id> 0

An example for Matter Door Lock device which has a `node id` set to ``1``, using the default enable key:

.. code-block:: console

   ./chip-tool generaldiagnostics test-event-trigger hex:00112233445566778899AABBCCDDEEFF 0x3277400000000000 1 0
