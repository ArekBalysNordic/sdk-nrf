.. _snippet_ot_usb:

OpenThread snippet for USB transport (ot-usb)
=============================================

.. contents::
   :local:
   :depth: 2

Overview
********

This snippet enables USB CDC ACM transport for the OpenThread stack.
Using this snippet, all communication with the OpenThread stack is done over USB.

.. note::

    This snippet is not supported on boards without a USB peripheral (for example, nRF54L15 DK).

Configuration
*************

To change the USB device product string, set the :kconfig:option:`CONFIG_CDC_ACM_SERIAL_PRODUCT_STRING` Kconfig option in your application's :file:`prj.conf` file.
