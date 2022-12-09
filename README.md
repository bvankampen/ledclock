# Led Clock
Led Clock based on a ESP8266. Built with VSCode and PlatformIO

![Led Clock Front](/docs/clock1.jpg)
![Led Clock Back](/docs/clock2.jpg)

## Parts list
- [NodeMCU](https://www.amazon.nl/AZDelivery-NodeMCU-Development-compatibel-Inclusief/dp/B0754LZ73Z/ref=sr_1_10?keywords=azdelivery+nodemcu&qid=1669240572&qu=eyJxc2MiOiIwLjAwIiwicXNhIjoiMC4wMCIsInFzcCI6IjAuMDAifQ%3D%3D&sprefix=azdelivery+node%2Caps%2C93&sr=8-10)
- [Led Matrix](https://www.amazon.nl/dp/B079HVW652/ref=pe_28126711_487102941_TE_SCE_dp_1?th=1)
- [RTC](https://www.amazon.nl/Precision-real-time-geheugenmodule-Arduino-Raspberry/dp/B07Y6B6VSC/ref=pd_rhf_d_ee_s_pd_sbs_rvi_sccl_1_1/258-9080716-9475458?pd_rd_w=CEW3w&content-id=amzn1.sym.8400a038-7c78-483d-b337-e3aa11c6dd2e&pf_rd_p=8400a038-7c78-483d-b337-e3aa11c6dd2e&pf_rd_r=7EFECAR8HP0EFN4PZ61Z&pd_rd_wg=q6hkZ&pd_rd_r=dc899db3-936a-4b26-b3ff-8e61ef79fd44&pd_rd_i=B07Y6B6VSC&psc=1)

## Schema
Included in Kicad folder
![Kicad](/docs/kicad.jpg)
![PCB](/docs/pcb.jpg)

## NodeMCU
- [pinout](https://cdn.shopify.com/s/files/1/1509/1638/files/NodeMCU_LUA_Amica_V2_Pinout_Diagram.pdf?14596320546790113351)
- [datasheet](https://cdn.shopify.com/s/files/1/1509/1638/files/NodeMCU_LUA_Amica_V2_Modul_mit_ESP8266_12E_Datenblatt.pdf?10729131952355562192)

## Libraries Used
- [RTCLib](https://github.com/adafruit/RTClib)
- [MD_MAX72XX](https://github.com/MajicDesigns/MD_MAX72XX)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)
