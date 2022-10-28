# DFRobot_ORP_PRO
DFRobot's DFRobot_ORP_PRO

## DFRobot_Trafficlight Library for Arduino
---------------------------------------------------------



## Table of Contents

* [Installation](#installation)
* [Methods](#methods)
* [Compatibility](#compatibility)
* [History](#history)
* [Credits](#credits)

<snippet>
<content>

## Installation

To use this library download the zip file, uncompress it to a folder named Installation.
Download the zip file first to use this library and uncompress it to a folder named Installation.

## Methods

```C++
    /*
     * @brief不提供校准值的初始化，默认参考电压为 +2480mV
     * @param NULL
     * @return 没有返回值
     */
    DFRobot_ORP_PRO(/* args */);
    /*
     * @brief:初始化，calibrate为校准偏移值，即实际参考电压=2480mV+calibration
     * 例：当 calibration = -10 时，参考电压为 +2470mV
     */
    DFRobot_ORP_PRO(int calibration);

    /*
     * @brief设置校准值
     * @param NULL
     * @return 没有返回值
     */
    float setCalibration(float voltage);

    /*
     * @brief获取当前校准值
     * @param NULL
     * @return 没有返回值
     */
    float getCalibration();

    /*
     * @brief根据已设置的参考偏置电压，返回计算的ORP值
     * 注意，输入与输出均为mV
     * @param NULL
     * @return 没有返回值
     */
     float getORP(float voltage);

    /*
     * @brief短接 ORP 输入即 0mV 时执行校准
     * @返回值为相对 +2480mV 的偏差值
     * @例：短接后输入 voltage = 2450mV, 则返回值为 -30
     * @param NULL
     * @return可以直接填入 setCalibration
     */
    float calibrate(float voltage);
```
## Compatibility

MCU                | Work Well | Work Wrong | Untested  | Remarks
------------------ | :----------: | :----------: | :---------: | -----
FireBeetle-ESP32  |      √       |             |            | 
FireBeetle-ESP8266|      √       |              |             | 
Mega2560  |      √       |             |            | 
Arduino uno |       √      |             |            | 
Leonardo  |      √       |              |             | 




## History

- 17,06, 2021 - Version 0.2 released.


## Credits

Written by PengKaixing(kaixing.peng@dfrobot.com), 2021. (Welcome to our [website](https://www.dfrobot.com/))