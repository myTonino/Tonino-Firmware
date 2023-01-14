[Tonino Serial Protocol](https://github.com/myTonino/Tonino-Firmware/blob/master/Tonino-Serial.md)
==============================================

The Tonino firmware implements a simple protocol that can be used to adjust basic settings of the [Tonino roast color meter.](http://my-tonino.com)

---

Serial Port Settings
--------------------

* baudrate: 115200 (Classic Tonino), 57600 (Tiny Tonino)
* bytesize: 8
* parity: none
* stopbits: 1

---

Command Language
----------------

The following [EBNF](http://en.wikipedia.org/wiki/Extended_Backus–Naur_Form) production rules define the general syntax of the command language:

	sep = " " ;
	newline = "\n" ;
	number = int | float ;
	values = number | values, sep, number ;
	cmdline = cmd, [ sep, values ], newline ;
	result = cmd, [ ":", values ], newline ;

Each `cmdline` that is successfully processed is answered by a `result` with the same `cmd` and a potentially empty list of `numbers`.

*Commands supported by all Toninos*

Commands `cmd =`  | Purpose
--- | ---
[`TONINO`](#TONINO) | return version information
[`SCAN`](#SCAN) | scan and return T-value
[`I_SCAN`](#I_SCAN) | scan and return internal value
[`II_SCAN`](#II_SCAN) | scan and return raw values
[`D_SCAN`](#D_SCAN) | scan unilluminated and return raw values
[`SETCAL`](#SETCAL) | store calibration values
[`SETSCALING`](#SETSCALING) | set scaling values
[`GETSCALING`](#GETSCALING) | get scaling values
[`SETBRIGHTNESS`](#SETBRIGHTNESS) | set brightness
[`GETBRIGHTNESS`](#GETBRIGHTNESS) | get brightness
[`RESETDEF`](#RESETDEF) | reset to defaults

*Additional commands supported only by the Classic Tonino*

Commands `cmd =`  | Purpose
--- | ---
[`GETCAL`](#GETCAL) | retrieve calibration values
[`SETSAMPLING`](#SETSAMPLING) | set sampling
[`GETSAMPLING`](#GETSAMPLING) | get sampling
[`SETCMODE`](#SETCMODE) | set color measure mode
[`GETCMODE`](#GETCMODE) | get color measure mode
[`SETLTDELAY`](#SETLTDELAY) | set delay between can-up and scan
[`GETLTDELAY`](#GETLTDELAY) | get delay between can-up and scan
[`SETCALINIT`](#SETCALINIT) | set check calibration at start
[`GETCALINIT`](#GETCALINIT) | get check calibration at start


*Additional commands supported only by the Tiny Tonino*

Commands `cmd =`  | Purpose
--- | ---
[`SETTARGET`](#SETTARGET) | set target and range
[`GETTARGET`](#GETTARGET) | return target and range
[`SETSCALE`](#SETSCALE) | set scale name
[`GETSCALE`](#GETSCALE) | get scale name
[`SETNAME`](#SETNAME) | set user name
[`GETNAME`](#GETNAME) | get user name
[`SETDFLIP`](#SETDFLIP) | set display flip (removed in v2.2.0 and later)
[`GETDFLIP`](#GETDFLIP) | get display flip (removed in v2.2.0 and later)

*Additional commands supported only by the Tiny Tonino firmware v2.2.0 or later*

Commands `cmd =`  | Purpose
--- | ---
[`SETDSCALE`](#SETDSCALE) | set default scale
[`SETDSCALE `](#SETDSCALE) | get default scale

---

Detailed Command Syntax
-----------------------

* **TONINO** <a name="TONINO"></a>  
   Get firmware version (major minor build)

    * *Arguments:* none

    * *Results:*

        value | type
        --- | ---
        major | `int`
        minor | `int`
        build | `int`

    * *Example:*

        request | reply
        --- | ---
        `TONINO\n` | `TONINO:1 0 1\n`

 * **SCAN** <a name="SCAN"></a>  
     Scan and return T-value

    * *Arguments:* none

    * *Results:*

        value | type
        --- | ---
        T-value | `int`

    * *Example:*

        request | reply
        --- | ---
        `SCAN\n` | `SCAN:58\n`

* **I_SCAN**  <a name="I_SCAN"></a>  
    Scan and return internal value

    * *Arguments:* none

    * *Results:*

        value | type
        --- | ---
        Internal value | `float`

    * *Example:*

        request | reply
        --- | ---
        `I_SCAN\n` | `I_SCAN:3.434770\n`

* **II_SCAN**  <a name="II_SCAN"></a>  
    Scan and return raw values

    * *Arguments:* none

    * *Results:*

        value | type
        --- | ---
        white | `float`
        red | `float`
        green | `float`
        blue | `float`
        T-value | `int`

    * *Example:*

        request | reply
        --- | ---
        `II_SCAN\n` | `II_SCAN:30330 0 0 8980 58\n`

* **D_SCAN**  <a name="D_SCAN"></a>  
    Scan unilluminated and return raw values

    * *Arguments:* none

    * *Results:*

        value | type
        --- | ---
        white | `float`
        red | `float`
        green | `float`
        blue | `float`

    * *Example:*

        request | reply
        --- | ---
        `D_SCAN\n` | `D_SCAN:30330 0 0 8980\n`


* **SETBRIGHTNESS**  <a name="SETBRIGHTNESS"></a>  
    Set brightness

    * *Arguments:*

        param | type
        --- | ---
        b | `int` [0,..,15]]

    * *Results:*:  none

    * *Example:* 

        request | reply
        --- | ---
        `SETBRIGHTNESS 10\n` | `SETBRIGHTNESS\n`

* **GETBRIGHTNESS**  <a name="GETBRIGHTNESS"></a>  
    Get brightness

    * *Arguments:* none

    * *Results:* 

        value | type
        --- | ---
        b | `int`     

    * *Example:* 

        request | reply
        --- | ---
        `GETBRIGHTNESS\n` | `GETBRIGHTNESS:10\n`

* **RESETDEF**  <a name="RESETDEF"></a>  
    Reset to defaults

    * *Arguments:* none

    * *Results:* none

    * *Example:*

        request | reply
        --- | ---
        `RESETDEF\n` | `RESETDEF\n`

* **SETCAL**  <a name="SETCAL"></a>  
    Set calibration values

    * *Arguments:*

        param | type
        --- | ---
        slope | `float`
        intercept | `float`

    * *Results:*: none

    * *Example:*

        request | reply
        --- | ---
        `SETCAL 1.024999 -0.032341\n` | `SETCAL\n`

* **GETCAL**  <a name="GETCAL"></a>  
    Get calibration values

    * *Arguments:* none

    * *Results:*:

        value | type
        --- | ---
        slope | `float`
        intercept | `float`

    * *Example:*

        request | reply
        --- | ---
        `GETCAL\n` | `GETCAL:1.024999 -0.032341\n`

* **SETSCALING**  <a name="SETSCALING"></a>  
    Set set scaling values

    * *Arguments:* ax^3 + bx^2 + cx + d

        param | type
        --- | ---
        a | `float`
        b | `float`
        c | `float`
        d | `float`

    * *Results:*: none

    * *Example:*

        request | reply
        --- | ---
        `SETSCALING 0.0 0.0 91.248359 -254.914581\n` | `SETSCALING\n`


* **GETSCALING**  <a name="GETSCALING"></a>  
    Get scaling values

    * *Arguments:*: none

    * *Results:* ax^3 + bx^2 + cx + d

        value | type
        --- | ---
        a | `float`
        b | `float`
        c | `float`
        d | `float`

    * *Example:*

        request | reply
        --- | ---
        `GETSCALING\n` | `GETSCALING:0.000000 0.000000 91.248359 -254.914581\n`

* **SETSAMPLING**  <a name="SETSAMPLING"></a>  
    Set sampling

    * *Arguments:*  sampling duration = 1sec / d

        param | type
        --- | ---
        d | `int` [1,..,255]

    * *Results:* none

    * *Example:*

        request | reply
        --- | ---
        `SETSAMPLING 5\n` | `SETSAMPLING\n`

* **GETSAMPLING**  <a name="GETSAMPLING"></a>  
    Get sampling

    * *Arguments:* none

    * *Results:* sampling duration = 1sec / d

        value | type
        --- | ---
        d | `int`

    * *Example:* 

        request | reply
        --- | ---
        `GETSAMPLING\n` | `GETSAMPLING:5\n`

* **SETCMODE**  <a name="SETCMODE"></a>  
    Set color measure mode

    * *Arguments:*  
       1 : white  
       2 : red  
       4 : green  
       8 : blue
       
        *e.g. 1+8=9 activates white and blue*  

        param | type
        --- | ---
        c | `int`

    * *Results:*:  none

    * *Example:*

        request | reply
        --- | ---
        `SETCMODE 9\n` | `SETCMODE\n`

* **GETCMODE**  <a name="GETCMODE"></a>  
    Get color measure mode

    * *Arguments:* none

    * *Results:*  
       1 : white  
       2 : red  
       4 : green  
       8 : blue
       
        *e.g. 1+8=9 returns white and blue*  

        value | type
        --- | ---
        c | `int`

    * *Example:* 

        request | reply
        --- | ---
        `GETCMODE\n` | `GETCMODE:9\n`

* **SETLTDELAY**  <a name="SETLTDELAY"></a>  
    Set delay between can-up and scan

    * *Arguments:* delay = d * 1/10sec

        param | type
        --- | ---
        d | `int` [0,..,255]

    * *Results:*:  none

    * *Example:* 

        request | reply
        --- | ---
        `SETCMODE 10\n` | `SETCMODE\n`

* **GETLTDELAY**  <a name="GETLTDELAY"></a>  
    Get delay between can-up and scan

    * *Arguments:* none

    * *Results:* delay = d * 1/10sec

        value | type
        --- | ---
        d | `int`

    * *Example:* 

        request | reply
        --- | ---
        `GETCMODE\n` | `GETCMODE:10\n`

* **SETCALINIT**  <a name="SETCALINIT"></a>  
    Set check calibration at start

    * *Arguments:*

        param | type
        --- | ---
        on | `int` (0 for false or 1 for true)

    * *Results:*  none

    * *Example:* 

        request | reply
        --- | ---
        `SETCALINIT 1\n` | `SETCALINIT\n`

* **GETCALINIT**  <a name="GETCALINIT"></a>  
    Get check calibration at start

    * *Arguments:* none

    * *Results:* 

        value | type
        --- | ---
        on | `int` (0 for false or 1 for true)

    * *Example:*

        request | reply
        --- | ---
        `GETCALINIT\n` | `GETCALINIT:1\n`

* **SETTARGET**  <a name="SETTARGET"></a>  
    Set set scaling values

    * *Arguments:*

        param | type
        --- | ---
        target | `int`
        range | `int`

    * *Results:*: none

    * *Example:*

        request | reply
        --- | ---
        `SETTARGET 100 5\n` | `SETTARGET\n`

* **GETTARGET**  <a name="GETTARGET"></a>  
    Get scaling values

    * *Arguments:*: none

    * *Results:*

        value | type
        --- | ---
        target | `int`
        range | `int`
    * *Example:*

        request | reply
        --- | ---
        `GETTARGET\n` | `GETTARGET:100 5\n`

* **SETSCALE**  <a name="SETSCALE"></a>  
    Set scale name

    * *Arguments:*

        param | type
        --- | ---
        name | `str`

    * *Results:*  none

    * *Example:* 

        request | reply
        --- | ---
        `SETSCALE Agtron\n` | `SETSCALE\n`

* **GETSCALE**  <a name="GETSCALE"></a>  
    Get scale name

    * *Arguments:* none

    * *Results:* 

        value | type
        --- | ---
        name | `str`

    * *Example:*

        request | reply
        --- | ---
        `GETSCALE\n` | `GETSCALE:Agtron\n`

* **SETNAME**  <a name="SETNAME"></a>  
    Set user name

    * *Arguments:*

        param | type
        --- | ---
        name | `str`

    * *Results:*  none

    * *Example:* 

        request | reply
        --- | ---
        `SETNAME Steve\n` | `SETNAME\n`

* **GETNAME**  <a name="GETNAME"></a>  
    Get user name

    * *Arguments:* none

    * *Results:* 

        value | type
        --- | ---
        name | `str`

    * *Example:*

        request | reply
        --- | ---
        `GETNAME\n` | `GETNAME:Steve\n`

* **SETDFLIP**  <a name="SETDFLIP"></a>  
    Set display flip mode

    * *Arguments:*

        param | type
        --- | ---
        on | `int` (0 for false or 1 for true)

    * *Results:*  none

    * *Example:* 

        request | reply
        --- | ---
        `SETDFLIP 1\n` | `SETDFLIP\n`

* **GETDFLIP**  <a name="GETDFLIP"></a>  
    Get display flip mode

    * *Arguments:* none

    * *Results:* 

        value | type
        --- | ---
        on | `int` (0 for false or 1 for true)

    * *Example:*

        request | reply
        --- | ---
        `GETDFLIP\n` | `GETDFLIP:1\n`

* **SETDSCALE**  <a name="SETDSCALE"></a>  
    Set default scale

    * *Arguments:*

        param | type
        --- | ---
        scale | `int` (0 for Tonino or 1 for Agtron)

    * *Results:*  none

    * *Example:* 

        request | reply
        --- | ---
        `SETDSCALE 1\n` | `SETDSCALE\n`

* **GETDSCALE**  <a name="GETDSCALE"></a>  
    Get default sale

    * *Arguments:* none

    * *Results:* 

        value | type
        --- | ---
        scale | `int` (0 for Tonino or 1 for Agtron)

    * *Example:*

        request | reply
        --- | ---
        `GETDSCALE\n` | `GETDSCALE:1\n`