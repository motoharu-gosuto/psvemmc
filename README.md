# psvemmc
emmc dumping driver, server and client

client - Windows client that connects to user app on PS Vita. 
         Allows to read emmc, game cart or memory cart on sector level (SceSdif API, SceMsif API)
         
server - User app on PS Vita. Acts as a server that provides some simple read/write proxy api for client.

driver - Driver module for PS Vita. Exposes real read/write API that utilizes kernel functions.
         This module also serves as entry point for R&D of kernel patch 
         that allows to hook SD card as Game Cart (one to one dump can be done with psvemmc or 
         psvcd and then can be written with dd).
         
error_logger - User module for PS Vita. Serves as entry point for R&D of SceShell patch that allows to hook SD card as Game Cart.
