EESchema Schematic File Version 4
LIBS:esion-lo-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L MCU_Microchip_ATtiny:ATtiny84-20SSU U3
U 1 1 5C63D8D4
P 6600 5100
F 0 "U3" H 6070 5146 50  0000 R CNN
F 1 "ATtiny84-20SSU" H 6070 5055 50  0000 R CNN
F 2 "Package_SO:SOIC-14_3.9x8.7mm_P1.27mm" H 6600 5100 50  0001 C CIN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/doc8006.pdf" H 6600 5100 50  0001 C CNN
	1    6600 5100
	-1   0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR0101
U 1 1 5C63F259
P 9500 1700
F 0 "#PWR0101" H 9500 1450 50  0001 C CNN
F 1 "GNDREF" H 9505 1527 50  0000 C CNN
F 2 "" H 9500 1700 50  0001 C CNN
F 3 "" H 9500 1700 50  0001 C CNN
	1    9500 1700
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR0103
U 1 1 5C63F355
P 3550 3950
F 0 "#PWR0103" H 3550 3700 50  0001 C CNN
F 1 "GNDREF" H 3555 3777 50  0000 C CNN
F 2 "" H 3550 3950 50  0001 C CNN
F 3 "" H 3550 3950 50  0001 C CNN
	1    3550 3950
	1    0    0    -1  
$EndComp
$Comp
L power:+3V8 #PWR0104
U 1 1 5C63F3CB
P 7450 1100
F 0 "#PWR0104" H 7450 950 50  0001 C CNN
F 1 "+3V8" H 7465 1273 50  0000 C CNN
F 2 "" H 7450 1100 50  0001 C CNN
F 3 "" H 7450 1100 50  0001 C CNN
	1    7450 1100
	1    0    0    -1  
$EndComp
$Comp
L Device:C C3
U 1 1 5C644550
P 9950 1500
F 0 "C3" H 9835 1454 50  0000 R CNN
F 1 "0.1uF" H 9835 1545 50  0000 R CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 9988 1350 50  0001 C CNN
F 3 "~" H 9950 1500 50  0001 C CNN
	1    9950 1500
	-1   0    0    1   
$EndComp
$Comp
L Device:C C4
U 1 1 5C644C6B
P 10400 1500
F 0 "C4" H 10515 1546 50  0000 L CNN
F 1 "10uF" H 10515 1455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 10438 1350 50  0001 C CNN
F 3 "~" H 10400 1500 50  0001 C CNN
	1    10400 1500
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5C648670
P 2500 3050
F 0 "R3" H 2430 3004 50  0000 R CNN
F 1 "10K" H 2430 3095 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 2430 3050 50  0001 C CNN
F 3 "~" H 2500 3050 50  0001 C CNN
	1    2500 3050
	-1   0    0    1   
$EndComp
Wire Wire Line
	8400 1650 8400 1700
$Comp
L Switch:SW_Push SW1
U 1 1 5C64D7AF
P 2350 2100
F 0 "SW1" H 2350 2385 50  0000 C CNN
F 1 "Reset" H 2350 2294 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_CK_RS282G05A3" H 2350 2300 50  0001 C CNN
F 3 "" H 2350 2300 50  0001 C CNN
	1    2350 2100
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW2
U 1 1 5C64D89E
P 2300 2900
F 0 "SW2" H 2300 3185 50  0000 C CNN
F 1 "Configure" H 2300 3094 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPST_CK_RS282G05A3" H 2300 3100 50  0001 C CNN
F 3 "" H 2300 3100 50  0001 C CNN
	1    2300 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5C6581D8
P 2700 1900
F 0 "R2" H 2770 1946 50  0000 L CNN
F 1 "10K" H 2770 1855 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 2630 1900 50  0001 C CNN
F 3 "~" H 2700 1900 50  0001 C CNN
	1    2700 1900
	1    0    0    -1  
$EndComp
$Comp
L Device:C C5
U 1 1 5C6582EF
P 2350 1650
F 0 "C5" V 2098 1650 50  0000 C CNN
F 1 "0.1uF" V 2189 1650 50  0000 C CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 2388 1500 50  0001 C CNN
F 3 "~" H 2350 1650 50  0001 C CNN
	1    2350 1650
	0    1    1    0   
$EndComp
$Comp
L power:GNDREF #PWR0105
U 1 1 5C65A0CF
P 2000 2150
F 0 "#PWR0105" H 2000 1900 50  0001 C CNN
F 1 "GNDREF" H 2005 1977 50  0000 C CNN
F 2 "" H 2000 2150 50  0001 C CNN
F 3 "" H 2000 2150 50  0001 C CNN
	1    2000 2150
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0106
U 1 1 5C675D20
P 10400 1200
F 0 "#PWR0106" H 10400 1050 50  0001 C CNN
F 1 "+3.3V" H 10415 1373 50  0000 C CNN
F 2 "" H 10400 1200 50  0001 C CNN
F 3 "" H 10400 1200 50  0001 C CNN
	1    10400 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 2100 2150 2100
Wire Wire Line
	3550 1650 2700 1650
Wire Wire Line
	2700 1650 2500 1650
Connection ~ 2700 1650
Wire Wire Line
	2200 1650 2000 1650
Wire Wire Line
	2000 1650 2000 2100
$Comp
L Device:LED_Small D1
U 1 1 5C697BBB
P 5600 3650
F 0 "D1" H 5450 3750 50  0000 L CNN
F 1 "Blue" H 5250 3750 50  0000 L CNN
F 2 "LED_SMD:LED_0805_2012Metric" V 5600 3650 50  0001 C CNN
F 3 "~" V 5600 3650 50  0001 C CNN
	1    5600 3650
	-1   0    0    -1  
$EndComp
$Comp
L Device:R R5
U 1 1 5C697D76
P 5350 3650
F 0 "R5" V 5250 3600 50  0000 R CNN
F 1 "220" V 5250 3800 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5280 3650 50  0001 C CNN
F 3 "~" H 5350 3650 50  0001 C CNN
	1    5350 3650
	0    1    1    0   
$EndComp
$Comp
L power:+3.3V #PWR0108
U 1 1 5C6B3CEA
P 2000 2700
F 0 "#PWR0108" H 2000 2550 50  0001 C CNN
F 1 "+3.3V" H 2015 2873 50  0000 C CNN
F 2 "" H 2000 2700 50  0001 C CNN
F 3 "" H 2000 2700 50  0001 C CNN
	1    2000 2700
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR0110
U 1 1 5C6DE940
P 6600 6150
F 0 "#PWR0110" H 6600 5900 50  0001 C CNN
F 1 "GNDREF" H 6605 5977 50  0000 C CNN
F 2 "" H 6600 6150 50  0001 C CNN
F 3 "" H 6600 6150 50  0001 C CNN
	1    6600 6150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 3050 5400 3050
Wire Wire Line
	2000 2900 2100 2900
$Comp
L power:GNDREF #PWR0114
U 1 1 5C788F05
P 2500 3200
F 0 "#PWR0114" H 2500 2950 50  0001 C CNN
F 1 "GNDREF" H 2505 3027 50  0000 C CNN
F 2 "" H 2500 3200 50  0001 C CNN
F 3 "" H 2500 3200 50  0001 C CNN
	1    2500 3200
	1    0    0    -1  
$EndComp
$Comp
L Device:R R17
U 1 1 5C796396
P 4650 5850
F 0 "R17" H 4720 5896 50  0000 L CNN
F 1 "10K" H 4720 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4580 5850 50  0001 C CNN
F 3 "~" H 4650 5850 50  0001 C CNN
	1    4650 5850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R18
U 1 1 5C79881E
P 4950 5850
F 0 "R18" H 5020 5896 50  0000 L CNN
F 1 "10K" H 5020 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4880 5850 50  0001 C CNN
F 3 "~" H 4950 5850 50  0001 C CNN
	1    4950 5850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R16
U 1 1 5C7BC067
P 4350 5850
F 0 "R16" H 4420 5896 50  0000 L CNN
F 1 "10K" H 4420 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 4280 5850 50  0001 C CNN
F 3 "~" H 4350 5850 50  0001 C CNN
	1    4350 5850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R15
U 1 1 5C7BC075
P 4050 5850
F 0 "R15" H 4120 5896 50  0000 L CNN
F 1 "10K" H 4120 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3980 5850 50  0001 C CNN
F 3 "~" H 4050 5850 50  0001 C CNN
	1    4050 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 6000 4350 6000
Wire Wire Line
	4650 6000 4950 6000
$Comp
L power:+3.3V #PWR0115
U 1 1 5C7F553E
P 2050 4450
F 0 "#PWR0115" H 2050 4300 50  0001 C CNN
F 1 "+3.3V" H 2065 4623 50  0000 C CNN
F 2 "" H 2050 4450 50  0001 C CNN
F 3 "" H 2050 4450 50  0001 C CNN
	1    2050 4450
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0116
U 1 1 5C83DA29
P 2700 1400
F 0 "#PWR0116" H 2700 1250 50  0001 C CNN
F 1 "+3.3V" H 2715 1573 50  0000 C CNN
F 2 "" H 2700 1400 50  0001 C CNN
F 3 "" H 2700 1400 50  0001 C CNN
	1    2700 1400
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR0119
U 1 1 5C97D6BC
P 5650 6000
F 0 "#PWR0119" H 5650 5750 50  0001 C CNN
F 1 "GNDREF" H 5655 5827 50  0000 C CNN
F 2 "" H 5650 6000 50  0001 C CNN
F 3 "" H 5650 6000 50  0001 C CNN
	1    5650 6000
	1    0    0    -1  
$EndComp
Wire Wire Line
	10400 1200 10400 1300
$Comp
L Switch:SW_SPDT SW4
U 1 1 5CC79E91
P 7750 1200
F 0 "SW4" H 7750 875 50  0000 C CNN
F 1 "POWER" H 7750 966 50  0000 C CNN
F 2 "Button_Switch_SMD:SW_SPDT_PCM12" H 7750 1200 50  0001 C CNN
F 3 "" H 7750 1200 50  0001 C CNN
	1    7750 1200
	1    0    0    1   
$EndComp
Connection ~ 9500 1700
$Comp
L Regulator_Linear:AP2112K-3.3 U1
U 1 1 5C63E145
P 9500 1400
F 0 "U1" H 9500 1742 50  0000 C CNN
F 1 "AP2112K-3.3" H 9500 1651 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 9500 1725 50  0001 C CNN
F 3 "https://www.diodes.com/assets/Datasheets/AP2112.pdf" H 9500 1500 50  0001 C CNN
	1    9500 1400
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5CCFFDEE
P 9050 1400
F 0 "R1" V 8843 1400 50  0000 C CNN
F 1 "10K" V 8934 1400 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 8980 1400 50  0001 C CNN
F 3 "~" H 9050 1400 50  0001 C CNN
	1    9050 1400
	0    -1   1    0   
$EndComp
Wire Wire Line
	8900 1400 8850 1400
Wire Wire Line
	8850 1400 8850 1300
Connection ~ 8850 1300
Wire Wire Line
	8850 1300 9200 1300
$Comp
L Device:D D3
U 1 1 5CD47AC4
P 8200 1300
F 0 "D3" H 8200 1516 50  0000 C CNN
F 1 "MBR120" H 8200 1425 50  0000 C CNN
F 2 "Diode_SMD:D_SOD-523" H 8200 1300 50  0001 C CNN
F 3 "~" H 8200 1300 50  0001 C CNN
	1    8200 1300
	-1   0    0    -1  
$EndComp
Wire Wire Line
	7450 1200 7550 1200
$Comp
L Device:Battery_Cell BT1
U 1 1 5CDDFC06
P 7450 1600
F 0 "BT1" H 7568 1696 50  0000 L CNN
F 1 "Battery_Cell" H 7568 1605 50  0000 L CNN
F 2 "Battery:BatteryHolder_Keystone_1042_1x18650" V 7450 1660 50  0001 C CNN
F 3 "~" V 7450 1660 50  0001 C CNN
	1    7450 1600
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 1100 7450 1200
Wire Wire Line
	7450 1400 7450 1200
Connection ~ 7450 1200
NoConn ~ 7950 1100
Wire Wire Line
	7950 1300 8050 1300
Wire Wire Line
	2700 1400 2700 1650
$Comp
L Device:CP C1
U 1 1 5D0EAEED
P 8400 1500
F 0 "C1" H 8518 1546 50  0000 L CNN
F 1 "10uF/10V" H 8518 1455 50  0000 L CNN
F 2 "Capacitor_Tantalum_SMD:CP_EIA-3528-21_Kemet-B" H 8438 1350 50  0001 C CNN
F 3 "~" H 8400 1500 50  0001 C CNN
	1    8400 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8400 1700 9500 1700
Wire Wire Line
	8400 1300 8400 1350
Wire Wire Line
	8350 1300 8400 1300
Connection ~ 8400 1300
Wire Wire Line
	8400 1300 8850 1300
Wire Wire Line
	9800 1300 9950 1300
Wire Wire Line
	10400 1350 10400 1300
Connection ~ 10400 1300
Wire Wire Line
	9950 1350 9950 1300
Connection ~ 9950 1300
Wire Wire Line
	9950 1300 10400 1300
Wire Wire Line
	9500 1700 9950 1700
Wire Wire Line
	10400 1650 10400 1700
Wire Wire Line
	9950 1650 9950 1700
Connection ~ 9950 1700
Wire Wire Line
	9950 1700 10400 1700
$Comp
L Device:R R14
U 1 1 5C6B1049
P 3750 5850
F 0 "R14" H 3820 5896 50  0000 L CNN
F 1 "10K" H 3820 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3680 5850 50  0001 C CNN
F 3 "~" H 3750 5850 50  0001 C CNN
	1    3750 5850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R13
U 1 1 5C6B105E
P 3450 5850
F 0 "R13" H 3520 5896 50  0000 L CNN
F 1 "10K" H 3520 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3380 5850 50  0001 C CNN
F 3 "~" H 3450 5850 50  0001 C CNN
	1    3450 5850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R11
U 1 1 5C6B1065
P 2850 5850
F 0 "R11" H 2920 5896 50  0000 L CNN
F 1 "10K" H 2920 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 2780 5850 50  0001 C CNN
F 3 "~" H 2850 5850 50  0001 C CNN
	1    2850 5850
	1    0    0    -1  
$EndComp
$Comp
L Device:R R12
U 1 1 5C6B106C
P 3150 5850
F 0 "R12" H 3220 5896 50  0000 L CNN
F 1 "10K" H 3220 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 3080 5850 50  0001 C CNN
F 3 "~" H 3150 5850 50  0001 C CNN
	1    3150 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 2500 2950 2500
Wire Wire Line
	5200 3650 4950 3650
Connection ~ 8400 1700
$Comp
L ESP32-footprints-Shem-Lib:ESP32-WROOM U2
U 1 1 5C710975
P 4500 2600
F 0 "U2" H 4475 3987 60  0000 C CNN
F 1 "ESP32-WROOM" H 4475 3881 60  0000 C CNN
F 2 "ESP32-footprints-Lib:ESP32-WROOM" H 4850 3950 60  0001 C CNN
F 3 "" H 4050 3050 60  0001 C CNN
	1    4500 2600
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x06_Male J2
U 1 1 5C903970
P 6700 2400
F 0 "J2" H 6672 2373 50  0000 R CNN
F 1 "ESP Flash" H 6672 2282 50  0000 R CNN
F 2 "Connector_JST:JST_XH_B06B-XH-A_1x06_P2.50mm_Vertical" H 6700 2400 50  0001 C CNN
F 3 "~" H 6700 2400 50  0001 C CNN
	1    6700 2400
	-1   0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0102
U 1 1 5C9BCFCE
P 6600 4200
F 0 "#PWR0102" H 6600 4050 50  0001 C CNN
F 1 "+3.3V" H 6615 4373 50  0000 C CNN
F 2 "" H 6600 4200 50  0001 C CNN
F 3 "" H 6600 4200 50  0001 C CNN
	1    6600 4200
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR0109
U 1 1 5C9E4C95
P 6400 2100
F 0 "#PWR0109" H 6400 1950 50  0001 C CNN
F 1 "+3.3V" H 6415 2273 50  0000 C CNN
F 2 "" H 6400 2100 50  0001 C CNN
F 3 "" H 6400 2100 50  0001 C CNN
	1    6400 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	7450 1700 8400 1700
Wire Wire Line
	6500 2700 6400 2700
Wire Wire Line
	6500 2200 6400 2200
Wire Wire Line
	6400 2200 6400 2100
Wire Wire Line
	6600 6000 6600 6150
Wire Wire Line
	3550 2000 3550 1650
Wire Wire Line
	2550 2100 2700 2100
Connection ~ 2000 2100
Wire Wire Line
	2000 2100 2000 2150
Wire Wire Line
	2000 2700 2000 2900
Wire Wire Line
	2750 2300 2750 5000
Wire Wire Line
	2850 2200 2850 4900
Wire Wire Line
	3050 2400 3050 4700
Wire Wire Line
	2950 2500 2950 4800
Wire Wire Line
	5600 2150 5600 2400
Wire Wire Line
	5700 2050 5700 2300
NoConn ~ 5400 2350
NoConn ~ 5400 2250
NoConn ~ 5400 1950
NoConn ~ 5400 1850
NoConn ~ 5400 2450
NoConn ~ 5400 2550
NoConn ~ 5400 2750
NoConn ~ 5400 2850
NoConn ~ 4650 3650
NoConn ~ 4550 3650
NoConn ~ 4250 3650
Wire Wire Line
	4350 6000 4650 6000
Connection ~ 4350 6000
Connection ~ 4650 6000
NoConn ~ 6000 4500
NoConn ~ 3550 2600
Wire Wire Line
	2700 1750 2700 1650
Wire Wire Line
	2700 2050 2700 2100
Connection ~ 2700 2100
$Comp
L Transistor_FET:BSS83P Q2
U 1 1 5CA66F64
P 9050 2900
F 0 "Q2" H 9255 2946 50  0000 L CNN
F 1 "BSS192P" H 9255 2855 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-89-3" H 9250 2825 50  0001 L CIN
F 3 "http://www.farnell.com/datasheets/1835997.pdf" H 9050 2900 50  0001 L CNN
	1    9050 2900
	1    0    0    -1  
$EndComp
$Comp
L Transistor_FET:BSS138 Q1
U 1 1 5CA670BA
P 8550 3100
F 0 "Q1" H 8755 3146 50  0000 L CNN
F 1 "BSS138" H 8755 3055 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 8750 3025 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/BS/BSS138.pdf" H 8550 3100 50  0001 L CNN
	1    8550 3100
	1    0    0    -1  
$EndComp
$Comp
L Device:R R7
U 1 1 5CA73651
P 8650 2750
F 0 "R7" H 8720 2796 50  0000 L CNN
F 1 "20K" H 8720 2705 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 8580 2750 50  0001 C CNN
F 3 "~" H 8650 2750 50  0001 C CNN
	1    8650 2750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R8
U 1 1 5CA7FF5B
P 8200 3100
F 0 "R8" V 8407 3100 50  0000 C CNN
F 1 "100" V 8316 3100 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 8130 3100 50  0001 C CNN
F 3 "~" H 8200 3100 50  0001 C CNN
	1    8200 3100
	0    -1   -1   0   
$EndComp
$Comp
L Device:R R9
U 1 1 5CAD6B5A
P 9150 3250
F 0 "R9" H 9220 3296 50  0000 L CNN
F 1 "4.7K" H 9220 3205 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 9080 3250 50  0001 C CNN
F 3 "~" H 9150 3250 50  0001 C CNN
	1    9150 3250
	1    0    0    -1  
$EndComp
$Comp
L Device:R R10
U 1 1 5CAD6C56
P 9150 3550
F 0 "R10" H 9220 3596 50  0000 L CNN
F 1 "20k" H 9220 3505 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 9080 3550 50  0001 C CNN
F 3 "~" H 9150 3550 50  0001 C CNN
	1    9150 3550
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR0112
U 1 1 5CAD6D24
P 9150 3700
F 0 "#PWR0112" H 9150 3450 50  0001 C CNN
F 1 "GNDREF" H 9155 3527 50  0000 C CNN
F 2 "" H 9150 3700 50  0001 C CNN
F 3 "" H 9150 3700 50  0001 C CNN
	1    9150 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	8850 2900 8650 2900
Connection ~ 8650 2900
Wire Wire Line
	8650 2600 8650 2550
Wire Wire Line
	8650 2550 9150 2550
Wire Wire Line
	9150 2550 9150 2700
Wire Wire Line
	8650 3300 8650 3700
Wire Wire Line
	8650 3700 9150 3700
Connection ~ 9150 3700
Text GLabel 8050 3100 0    50   Input ~ 0
IO25
Text GLabel 3550 2800 0    50   Output ~ 0
IO25
Text GLabel 9400 3400 2    50   Output ~ 0
IO27
Wire Wire Line
	9400 3400 9150 3400
Connection ~ 9150 3400
Text GLabel 3550 3000 0    50   Input ~ 0
IO27
Wire Wire Line
	3550 3300 3550 3650
Wire Wire Line
	3550 3650 4050 3650
Wire Wire Line
	2550 4650 2550 5200
Wire Wire Line
	2850 6000 3150 6000
Connection ~ 3150 6000
Wire Wire Line
	3150 6000 3450 6000
Connection ~ 3450 6000
Wire Wire Line
	3450 6000 3750 6000
Wire Wire Line
	3750 6000 4050 6000
Connection ~ 3750 6000
Connection ~ 4050 6000
Wire Wire Line
	2850 5700 2850 5000
Wire Wire Line
	3150 4900 3150 5700
Wire Wire Line
	3450 5700 3450 4800
Wire Wire Line
	3750 5700 3750 4700
Wire Wire Line
	4050 5700 4050 5600
Wire Wire Line
	4350 5700 4350 5500
Wire Wire Line
	4650 5700 4650 5400
Wire Wire Line
	4950 5700 4950 5200
Connection ~ 2850 5000
Wire Wire Line
	2850 5000 4150 5000
Connection ~ 3150 4900
Wire Wire Line
	3150 4900 6000 4900
Connection ~ 3450 4800
Wire Wire Line
	3450 4800 6000 4800
Connection ~ 3750 4700
Wire Wire Line
	3750 4700 6000 4700
Connection ~ 4050 5600
Wire Wire Line
	4050 5600 6000 5600
Connection ~ 4350 5500
Wire Wire Line
	4350 5500 6000 5500
Connection ~ 4650 5400
Wire Wire Line
	4650 5400 6000 5400
Connection ~ 4950 5200
Wire Wire Line
	4950 5200 6000 5200
Wire Wire Line
	3550 3100 3150 3100
Wire Wire Line
	3150 3100 3150 4900
Wire Wire Line
	6000 5100 5250 5100
Wire Wire Line
	3250 5100 3250 3200
Wire Wire Line
	3250 3200 3550 3200
Wire Wire Line
	4150 3650 4150 5000
Connection ~ 4150 5000
Wire Wire Line
	4150 5000 6000 5000
Wire Wire Line
	5500 5700 5500 4600
Wire Wire Line
	5500 5700 5650 5700
$Comp
L Device:R R19
U 1 1 5CE939E8
P 5250 5850
F 0 "R19" H 5320 5896 50  0000 L CNN
F 1 "R" H 5320 5805 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5180 5850 50  0001 C CNN
F 3 "~" H 5250 5850 50  0001 C CNN
	1    5250 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5250 6000 4950 6000
Connection ~ 5250 6000
Connection ~ 4950 6000
Wire Wire Line
	5250 5700 5250 5100
Connection ~ 5250 5100
Wire Wire Line
	5250 5100 3250 5100
Wire Wire Line
	5600 2500 5600 3050
NoConn ~ 3550 2700
NoConn ~ 4450 3650
NoConn ~ 4350 3650
NoConn ~ 5400 2650
NoConn ~ 6000 4600
Wire Wire Line
	4850 4600 5500 4600
Wire Wire Line
	4850 3650 4850 4600
$Comp
L Device:R R6
U 1 1 5CF5A3A4
P 5700 3450
F 0 "R6" V 5800 3550 50  0000 C CNN
F 1 "220" V 5800 3400 50  0000 C CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5630 3450 50  0001 C CNN
F 3 "~" H 5700 3450 50  0001 C CNN
	1    5700 3450
	0    -1   -1   0   
$EndComp
$Comp
L Device:LED_Small D2
U 1 1 5CF793B7
P 5950 3450
F 0 "D2" H 5850 3550 50  0000 C CNN
F 1 "Red" H 5700 3550 50  0000 C CNN
F 2 "LED_SMD:LED_0805_2012Metric" V 5950 3450 50  0001 C CNN
F 3 "~" V 5950 3450 50  0001 C CNN
	1    5950 3450
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3450 1100 3450 2100
Wire Wire Line
	5400 2150 5600 2150
Wire Wire Line
	5600 2500 6500 2500
Wire Wire Line
	5800 1100 5800 2600
Wire Wire Line
	5600 2400 6500 2400
Wire Wire Line
	5700 2050 5400 2050
Wire Wire Line
	5700 2300 6500 2300
Wire Wire Line
	5800 1100 3450 1100
Wire Wire Line
	5800 2600 6500 2600
Connection ~ 3450 2100
Wire Wire Line
	3450 2100 3550 2100
Wire Wire Line
	2700 2100 3450 2100
Wire Wire Line
	5400 3150 6400 3150
Connection ~ 6400 3150
Wire Wire Line
	6400 3150 6400 3250
Connection ~ 6400 3250
Wire Wire Line
	5400 3250 6400 3250
Wire Wire Line
	6400 3250 6400 3450
Wire Wire Line
	5700 3650 6400 3650
Connection ~ 6400 3650
Wire Wire Line
	3550 3650 3550 3950
Wire Wire Line
	3550 3950 6400 3950
Connection ~ 3550 3650
Connection ~ 3550 3950
Wire Wire Line
	3050 2400 3550 2400
Wire Wire Line
	3050 4700 3750 4700
Wire Wire Line
	2950 4800 3450 4800
Wire Wire Line
	2850 2200 3550 2200
Wire Wire Line
	2850 4900 3150 4900
Wire Wire Line
	2750 2300 3550 2300
Wire Wire Line
	2750 5000 2850 5000
Connection ~ 2500 2900
Wire Wire Line
	2500 2900 3550 2900
Wire Wire Line
	2350 5500 2350 5150
Wire Wire Line
	2250 5600 2250 5400
$Comp
L Device:R R20
U 1 1 5D1BD58D
P 5500 4450
F 0 "R20" H 5430 4496 50  0000 R CNN
F 1 "220" H 5430 4405 50  0000 R CNN
F 2 "Resistor_SMD:R_0805_2012Metric" V 5430 4450 50  0001 C CNN
F 3 "~" H 5500 4450 50  0001 C CNN
	1    5500 4450
	-1   0    0    -1  
$EndComp
Connection ~ 5500 4600
$Comp
L power:+3.3V #PWR0107
U 1 1 5D1BD709
P 5500 4300
F 0 "#PWR0107" H 5500 4150 50  0001 C CNN
F 1 "+3.3V" H 5515 4473 50  0000 C CNN
F 2 "" H 5500 4300 50  0001 C CNN
F 3 "" H 5500 4300 50  0001 C CNN
	1    5500 4300
	1    0    0    -1  
$EndComp
$Comp
L Device:C C2
U 1 1 5D1C470A
P 5650 5850
F 0 "C2" H 5765 5896 50  0000 L CNN
F 1 "0.1uF" H 5765 5805 50  0000 L CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 5688 5700 50  0001 C CNN
F 3 "~" H 5650 5850 50  0001 C CNN
	1    5650 5850
	1    0    0    -1  
$EndComp
Connection ~ 5650 5700
Wire Wire Line
	5650 5700 6000 5700
Wire Wire Line
	5650 6000 5250 6000
Connection ~ 5650 6000
Wire Wire Line
	6400 2700 6400 3150
$Comp
L Connector:Screw_Terminal_01x02 J6
U 1 1 5D208F60
P 1700 4900
F 0 "J6" H 2000 4900 50  0000 C CNN
F 1 "Connector 2" H 2000 4800 50  0000 C CNN
F 2 "TerminalBlock_MetzConnect:TerminalBlock_MetzConnect_Type059_RT06302HBWC_1x02_P3.50mm_Horizontal" H 1700 4900 50  0001 C CNN
F 3 "~" H 1700 4900 50  0001 C CNN
	1    1700 4900
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J7
U 1 1 5D20EA2D
P 1700 5150
F 0 "J7" H 2000 5150 50  0000 C CNN
F 1 "Connector 3" H 2000 5050 50  0000 C CNN
F 2 "TerminalBlock_MetzConnect:TerminalBlock_MetzConnect_Type059_RT06302HBWC_1x02_P3.50mm_Horizontal" H 1700 5150 50  0001 C CNN
F 3 "~" H 1700 5150 50  0001 C CNN
	1    1700 5150
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Screw_Terminal_01x02 J8
U 1 1 5D214504
P 1700 5400
F 0 "J8" H 2000 5400 50  0000 C CNN
F 1 "Connector 4" H 2000 5300 50  0000 C CNN
F 2 "TerminalBlock_MetzConnect:TerminalBlock_MetzConnect_Type059_RT06302HBWC_1x02_P3.50mm_Horizontal" H 1700 5400 50  0001 C CNN
F 3 "~" H 1700 5400 50  0001 C CNN
	1    1700 5400
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2450 4900 2450 5400
Wire Wire Line
	2550 5200 4950 5200
Wire Wire Line
	2450 5400 4650 5400
Wire Wire Line
	2350 5500 4350 5500
Wire Wire Line
	2250 5600 4050 5600
$Comp
L Connector:Screw_Terminal_01x02 J5
U 1 1 5D1E8CE5
P 1700 4650
F 0 "J5" H 2000 4650 50  0000 C CNN
F 1 "Connector 1" H 2000 4550 50  0000 C CNN
F 2 "TerminalBlock_MetzConnect:TerminalBlock_MetzConnect_Type059_RT06302HBWC_1x02_P3.50mm_Horizontal" H 1700 4650 50  0001 C CNN
F 3 "~" H 1700 4650 50  0001 C CNN
	1    1700 4650
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2050 4450 2050 4750
Wire Wire Line
	2050 5500 1900 5500
Wire Wire Line
	2250 5400 1900 5400
Wire Wire Line
	1900 5150 2350 5150
Wire Wire Line
	2450 4900 1900 4900
Wire Wire Line
	1900 4650 2550 4650
Wire Wire Line
	1900 4750 2050 4750
Connection ~ 2050 4750
Wire Wire Line
	2050 4750 2050 5000
Wire Wire Line
	1900 5000 2050 5000
Connection ~ 2050 5000
Wire Wire Line
	2050 5000 2050 5250
Wire Wire Line
	1900 5250 2050 5250
Connection ~ 2050 5250
Wire Wire Line
	2050 5250 2050 5500
Wire Wire Line
	6400 3650 6400 3950
Wire Wire Line
	6050 3450 6400 3450
Connection ~ 6400 3450
Wire Wire Line
	6400 3450 6400 3650
Wire Wire Line
	5550 3450 5500 3450
Wire Wire Line
	5500 3450 5500 2950
Wire Wire Line
	5500 2950 5400 2950
NoConn ~ 4750 3650
Text GLabel 8400 1050 1    50   Input ~ 0
M1
Text GLabel 9150 2400 1    50   Output ~ 0
M1
Wire Wire Line
	9150 2400 9150 2550
Connection ~ 9150 2550
Wire Wire Line
	8400 1050 8400 1300
$EndSCHEMATC
