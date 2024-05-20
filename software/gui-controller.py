## Cod adapted from https://github.com/pyimgui/pyimgui/blob/master/doc/examples/integrations_glfw3.py

from imgui.integrations.glfw import GlfwRenderer
import OpenGL.GL as gl
import glfw
import imgui
import sys
import struct
from serial import Serial
import time
import glob

ser = None

balls_per_minute = 10
shots = []

window_size = [500, 655]

def send_packet(id, contents):
    global ser
    if ser is None or not ser.isOpen():
        return

    if len(contents) > 5:
        print("malformed contents")
    else:
        # pad out empty fields
        try:
            contents = contents + [0] * (5 - len(contents))
            s = "".join([("b" if x < 0 else "B") for x in contents])
            ser.write(struct.pack(f">BB{s}B", 0x47, id, *contents, 0xED))
        except Exception:
            print("serial disconnected")
            ser = None

imgui.create_context()

if not glfw.init():
    print("Could not initialize OpenGL context")
    sys.exit(1)

glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, gl.GL_TRUE)

window = glfw.create_window(window_size[0], window_size[1], "gui-controller", None, None)
glfw.make_context_current(window)

if not window:
    glfw.terminate()
    print("Could not initialize Window")
    sys.exit(1)

impl = GlfwRenderer(window)

shot_to_add = [
    0, # yaw
    0, # bottom
    0, # left
    0  # right
]

serial_ports = []
serial_port_index = 0
serial_port = ""
keepalive_time = 0

while not glfw.window_should_close(window):
    glfw.poll_events()
    impl.process_inputs()

    imgui.new_frame()

    if time.time() - keepalive_time > 1:
        send_packet(4, [])
        keepalive_time = time.time()

    imgui.set_next_window_position(0,0)
    imgui.set_next_window_size(window_size[0],window_size[1])
    imgui.begin("window", True, flags=imgui.WINDOW_NO_RESIZE|imgui.WINDOW_NO_COLLAPSE|imgui.WINDOW_NO_TITLE_BAR|imgui.WINDOW_NO_MOVE)

    serial_changed, serial_port_index = imgui.combo(
        "Serial port", serial_port_index, serial_ports
    )
    if serial_changed:
        if ser and ser.isOpen():
            ser.close()
        serial_port = serial_ports[serial_port_index]
        try:
            ser = Serial(serial_port, 115200)
        except Exception:
            print("bad port")
    imgui.same_line()
    if imgui.button("Refresh"):
        serial_ports = glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*") + glob.glob("/dev/ttyS*")
    if ser and ser.isOpen():
        imgui.extra.text_ansi_colored("Connected", 0.4, 1.0, 0.4)
    else:
        imgui.extra.text_ansi_colored("Disconnected", 1.0, 0.4, 0.4)


    imgui.separator()

    _, balls_per_minute = imgui.slider_int(
        "Balls per minute", balls_per_minute, 1, 50,
    )

    if imgui.button(label="Start session"):
        send_packet(2, [balls_per_minute])
    if imgui.button(label="Stop session"):
        send_packet(3, [])

    imgui.separator()
    imgui.text("Configured shots:")
    for i, [yaw, bottom, left, right] in enumerate(shots):
        imgui.separator()
        imgui.text(f"Shot {i+1}:")
        imgui.text(f"Yaw angle (deg): {yaw}")
        imgui.text(f"Motor speeds: {bottom} (bottom), {left} (left), {right} (right)")
        if imgui.button(label=f"Remove shot##{i}"):
            send_packet(1, [i])
            shots.pop(i)

    imgui.separator()

    imgui.text("Add new shot:")
    _, shot_to_add[0] = imgui.slider_int(
        "Yaw angle (deg)", shot_to_add[0], -30, 30,
    )
    for i,m in enumerate(["Bottom", "Left", "Right"]):
        _, shot_to_add[i+1] = imgui.slider_int(
            f"{m} motor speed (%)", shot_to_add[i+1], 0, 100
            )

    if imgui.button(label="Add shot"):
        shots.append(shot_to_add.copy())
        send_packet(0, shot_to_add.copy())
        print("add shot")


    imgui.end()

    gl.glClearColor(1.0, 1.0, 1.0, 1)
    gl.glClear(gl.GL_COLOR_BUFFER_BIT)

    imgui.render()
    impl.render(imgui.get_draw_data())
    glfw.swap_buffers(window)

impl.shutdown()
glfw.terminate()