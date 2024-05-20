# Assembly instructions

## Ball Pusher

1. Solder four ribbon cable wires to the TCRT5000 sensor and glue it to the `Parallax servo holder`.

![](images/2.jpg)
![](images/3.jpg)
![](images/4.jpg)

2. Mount the Parallax continuous servomotor to the `Parallax servo holder` using four M3x16 bolts with nuts. Two bolts are sufficient.

![](images/5.jpg)

3. Pressure fit the cross-shaped connector provided with the Parallax continous servo into the `Crank`. Use glue if the fit is loose.

![](images/6.jpg)

4. Secure the cross connector and the `Crank` to the servomotor using the screw provided with the servomotor.

![](images/45.jpg)

5. Pressure fit the `Parallax servo holder` into `Housing 1`. Place the `Ball stopper` into `Housing 1` as indicated by the red square.

![](images/8.jpg)

6. Slide the `Piston` sideways into `Housing 1`.

![](images/9.jpg)

7. Attach `Housing 2` to `Housing 1`.

![](images/10.jpg)

8. Twist the `Piston` counterclockwise so it is parallel to the sliding surface.

![](images/11.jpg)

9. Attach the `Connecting rod` to the `Crank` and `Piston`.

![](images/12.jpg)

10. Attach the `Crank retaining pin` to the `Crank` and the `Piston retaining pin` to the `Piston`.

![](images/13.jpg)

11. Secure `Housing 1` and `Housing 2` together using four M3x20 bolts. Nuts are not required as the holes' inner diameter is smaller inside.

![](images/14.jpg)

12. Attach a rubber band to hold the `Ball stopper` upright.

![](images/15.jpg)

## Pipe

1. Secure `Angled connector pipe 1` and `Angled connector pipe 2` together using two M3x12 bolts and nuts on both sides.

![](images/17.jpg)

2. Secure `Straight connector pipe top` and `Angled connector pipe 1` together using two M3x12 bolts and nuts on both sides.

![](images/18.jpg)

3. Secure `Straight connector pipe top` and `Straight connector pipe bottom` together using two M3x12 bolts and nuts on both sides.

![](images/19.jpg)

## Launch Unit

1. Make three traction wheels with `Traction wheel mould inner` and `Traction wheel mould outer` using polysiloxane. I recommend Shore 20-25 A hardness polysiloxane.

![](images/46.jpg)

2. For each of the three BLDC motors: attach the `Motor guard`, stretch polysiloxane traction wheel around the `Motor guard`, and secure it using flange nut provided with the motor.

![](images/20.jpg)
![](images/23.jpg)

3. Pressure fit the arm-shaped connector that came with the TG90 servomotor into the `Servo hook`. Use glue if the fit is loose.

![](images/24.jpg)

4. Mount the servomotor into the `Launch Unit` while connecting the servomotor to the arm-shaped connector. Secure it using two screws provided with the servomotor. **Note:** I recommend connecting the servomotor and `Servo hook` later if the position of the servomotor is unknown. 

![](images/25.jpg)
![](images/26.jpg)

5. Attach the three BLDC motors to the `Launch Unit` using six M3x8 bolts. Adjust the pinching distance as necessary. I recommend trying to minimise the pinching distance as much as possible with all three traction wheels still making contact with a table tennis ball.

![](images/27.jpg)
![](images/28.jpg)

6. Solder four ribbon cable wires to the TCRT5000 sensor and glue it to the `Launch Unit`.

![](images/42.jpg)

## Pivot Mechanism

The assembly images show the Pivot Mechanism already connected to the Fixture but this is not required.

1. Place two `Bearing housing` on the `Housing` and lightly screw in four M3x12 bolts into the `Housing` but do not tighten them yet.

![](images/30.jpg)

2. Secure the servomotor to the `Housing` using the provided screws and mount the provided cross-shaped connector to it.

![](images/33.jpg)

3. Fit the 15x32x9 metal ball bearing between the two `Bearing housing`s and tighten them together using two M3x12 bolts and nuts. Tighten the four M3x12 bolts placed in step 1. Pressure fit the  `Shaft` into the ball bearing. Pressure fit the `Driver gear` into the arm-shaped cross of the servomotor. **Note:** I recommend pressure fitting the `Driven gear` into the `Shaft` later as the servomotor's initial position is unknown.

![](images/35.jpg)

## Fixture

1.
    1. Slide and secure the first `Pusher and Pipe connector` into the `Rod` using the first `Rectangular dowel`.
    2. Slide and secure the Pivot Mechanism into the `Rod` using the `Square dowel`.
    3. Slide the `Rod` into the `Clamp` and secure it using two `Round dowel`s. 
    4. Slide and secure the first `Pusher and Pipe connector` into the `Rod` using the first `Rectangular dowel`.
    5. Turn the `Bolt` into the `Clamp`. The `Bolt pad` can be used between the `Bolt` and a table's underside when mounting.

![](images/36.jpg)

2. Secure the Ball Pusher to the Fixture using four (one side depicted) M3x12 bolts.

![](images/37.jpg)

3. Secure the Launch Unit to the Pivot Mechanism's `Shaft` using two M3x12 bolts. **Note:** Before securing the Launch Unit, find the zero position of the Pivot Mechanism's servomotor by powering the robot on. The firmware rotates the servomotor to its midway position (0 degree yaw angle) on startup. Afterwards, pressure fit the Pivot Mechanism's `Driven gear` into the `Shaft` and then secure the Launch Unit.

![](images/38.jpg)
![](images/39.jpg)

4. Secure the Pipe to the Fixture using four M3x12 bolts. Secure the Pipe to the Ball Pusher using two M3x8 bolts.

![](images/41.jpg)
![](images/40.jpg)

## Electronics

Hot glue the ESC's wires to prevent damage. I recommend also wrapping them using electrical tape.

![](images/21.jpg)

Connect the components according to the following schematic:

![](images/schematic.png)

Although not strictly required I recommend using a protoboard for wiring the 5 V & 3.3 V power lines and for wiring the four resistors required by both IR reflective sensors.

The `Enclosure` can be used to house all the electronic components:

![](images/enclosure.png)

The protoboard and ESP32 module are secured to the enclosure using 6 M3x8 bolts.

The different wirings can be tied together using cable ties and electrical tape to form a harness. Alternatively, sleeving or wire loom can be used.

![](images/44.jpg)