# About

This library provides an interface for listening for multitouch inputs on windows and linux using java.
It treats mouse, pen and finger the same way.

## Windows

### Building

```cmd
gradlew.bat buildWindowsNative
```

## Linux

Uses the XInput2 touch protocol (`XI_TouchBegin`/`XI_TouchUpdate`/`XI_TouchEnd`) for X11. 
Unlike the Windows version it only reports true touch events (no mouse/pen), and since X11 touch events carry no button state,
`button` is always reported as `BUTTON_1`. You pass it the X11 window id (`Window`/`XID`) of the
window to listen on.

### Building

The `native-linux` folder builds `libMtLinux64.so` inside Docker, so the result doesn't depend on
whatever toolchain happens to be on your machine. Requires Docker (Docker Desktop on Windows, with
either the WSL2 backend or Linux containers enabled, works fine):

```bash
cd native-linux
./build.sh
```

This produces `native-linux/build/libMtLinux64.so`, which `./gradlew :java:processResources`
(or the `buildLinuxNative` Gradle task, which runs the script for you) picks up and bundles into
the jar alongside the Windows DLLs.


## Release
The native libs and jar binary is available at the maven repo:

```xml
<repository>
    <id>dev-core-public</id>
    <name>Public dev-core repo</name>
    <url>https://dev-core.org/maven-repo-pub</url>
</repository>

<dependency>
    <groupId>org.devcore</groupId>
    <artifactId>win-multitouch</artifactId>
    <version>1.0</version>
</dependency>
``` 

```groovy
allprojects {
    repositories {
        mavenLocal()
        jcenter()
        maven {
            name "dev-core-public"
            url "https://dev-core.org/maven-repo-pub/"
        }
    }
}

dependencies {
    implementation 'org.devcore:java-multitouch-native:1.0'
}
```
