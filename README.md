# ioctl-linux
Simple tool to call ioctl from command-line (for Linux).

Usage: `ioctl [file|device] [request] [string|number|struct...]`

The argument type is automatically parsed like so:
  - string: `Xsome stringX` (must start and end with any same non-numeric symbol)
  - struct: `64B` (indicating the size in bytes of the struct)
  - number: `10ull` (use any regular integer literal, defaults to int or float)

Note: On-the-fly compilation is used to resolve the request parameter if it is
not formatted as an unsigned long. This will:
 - execute a recursive grep on /usr/include to find the request definition in the header file;
 - create and compile a /tmp/XXXXXXXX.c file (using gcc);
 - run the compiled executable in order to find the unsigned long int value;
 - generate a warning (in stderr) with the unsigned long int value which may be
   used for - more efficient - subsequent calls.

In order to find out the actual unsigned long int value without
executing ioctl, use - as only request argument. In this case,
the unsigned long int value will be directly written to stdout.

Example usage:

Note the struct definition <linux/videodev2.h>:
```
  struct v4l2_capability {
      __u8    driver[16];
      __u8    card[32];
      __u8    bus_info[32];
      __u32   version;
      __u32   capabilities;
      __u32   device_caps;
      __u32   reserved[3];
  };
```

For efficiency, resolve the request parameter to an unsigned long:
```
  > ioctl /dev/video0 VIDIOC_QUERYCAP -
  2154321408
```

Use dd to extract the v4l2_capability.card string:
```
  > ioctl /dev/video0 2154321408 104B | dd bs=1 skip=16 count=32 2>&-; echo
  USB2.0 HD UVC WebCam: USB2.0 HD
```

Alternatively inefficiently but directly with an unknown buffer size:
```
  > ioctl /dev/video0 VIDIOC_QUERYCAP 10000B | dd bs=8 skip=2 count=4 2>&-; echo
  warning: Used on-the-fly compilation to resolve request parameter
  (VIDIOC_QUERYCAP). To optimize, run:
  ./ioctl /dev/video0 2154321408 10000B
  USB2.0 HD UVC WebCam: USB2.0 HD
```
