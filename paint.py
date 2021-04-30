
from tkinter import *
from PIL import Image
import json

config_file = 'config.json'

class Paint():

    def __init__(self):
        self.tk = Tk()
        self.tk.title('Map Creation')

        self.read_config(config_file)

        self.save_button = Button(self.tk, text='Save', command=self.save)
        self.save_button.grid(row=0, column=0)

        self.erase_button = Button(self.tk, text='Clear', command=self.erase)
        self.erase_button.grid(row=0, column=1)

        self.brush_button = Button(self.tk, text='Brush', command=self.brush)
        self.brush_button.grid(row=0, column=2)

        self.choose_size_button = Scale(self.tk, from_=1, to=10, orient=HORIZONTAL)
        self.choose_size_button.grid(row=0, column=3)

        self.start_button = Button(self.tk, text='Select Start', command=self.start)
        self.start_button.grid(row=2, column=1)

        self.goal_button = Button(self.tk, text='Select Goal', command=self.goal)
        self.goal_button.grid(row=2, column=2)

        self.canvas = Canvas(self.tk, bg='white', width=self.config['screen_width'], height=self.config['screen_height'])
        self.canvas.grid(row=1, columnspan=5)

        self.start_rect = None
        self.goal_rect = None

        self.setup()
        self.tk.mainloop()

    def rgb_to_hex(self, r, g, b):
        return '#%02x%02x%02x' % (r, g, b)

    def read_config(self, filename):
        cf = open(filename)
        self.config = json.load(cf)

    def write_json(self, filename):
        with open(filename, 'w') as f:
            json.dump(self.config, f, indent=4)

    def setup(self):
        self.old_x = None
        self.old_y = None

        self.color = self.config['default_brush_color']

        # set default brush size and line width from it
        self.choose_size_button.set(self.config['default_brush_size'])
        self.line_width = self.choose_size_button.get()

        # set default active button and make it 'sink' to indicate it
        self.active_button = self.brush_button
        self.active_button.config(relief=SUNKEN)

        # <B1-Motion>: mouse is moved with left mouse button held down
        self.canvas.bind('<B1-Motion>', self.paint)
        # <ButtonRelease-1>: left mouse button was released
        self.canvas.bind('<ButtonRelease-1>', self.reset)
        #
        self.canvas.bind('<ButtonRelease-3>', self.select)

    def save(self, filename='map'):
        self.canvas.postscript(file=filename + '.eps')

        image = Image.open(filename + '.eps')
        image.save(filename + '.png', 'png')

        print('Image was saved successfully.')
        quit()

    def erase(self):
        self.canvas.delete('all')
        self.start_rect['rectangle'] = self.create_rectangle_node(self.start_rect['event'],
                                        self.start_rect['node_size'], self.start_rect['color_hex'])
        self.goal_rect['rectangle'] = self.create_rectangle_node(self.goal_rect['event'],
                                        self.goal_rect['node_size'], self.goal_rect['color_hex'])

    def brush(self):
        self.activate_button(self.brush_button)

    def start(self):
        print('right click to select start location')
        self.activate_button(self.start_button)

    def goal(self):
        print('right click to select goal location')
        self.activate_button(self.goal_button)

    def activate_button(self, button):
        # raise currently active button and sink newly activated button
        self.active_button.config(relief=RAISED)

        button.config(relief=SUNKEN)
        self.active_button = button

    def create_rectangle_node(self, event, node_size, color_hex):
        return self.canvas.create_rectangle(event.x - node_size, event.y - node_size,
                                            event.x + node_size, event.y + node_size,
                                            fill=color_hex)

    def select(self, event):
        node_size = self.config['node_size']
        if self.active_button.config('text')[-1] == 'Select Start':
            print(self.start_rect)
            if self.start_rect:
                print('inside start if')
                self.canvas.delete(self.start_rect['rectangle'])
            start_color = self.config['start_color']
            start_color_hex = self.rgb_to_hex(start_color['r'], start_color['g'], start_color['b'])
            self.start_rect = {
                'rectangle': self.create_rectangle_node(event, node_size, start_color_hex),
                'event': event,
                'node_size': node_size,
                'color_hex': start_color_hex
            }

            self.config['start_position']['x'] = event.x
            self.config['start_position']['y'] = event.y
        elif self.active_button.config('text')[-1] == 'Select Goal':
            if self.goal_rect:
                self.canvas.delete(self.goal_rect['rectangle'])
            goal_color = self.config['goal_color']
            goal_color_hex = self.rgb_to_hex(goal_color['r'], goal_color['g'], goal_color['b'])
            self.goal_rect = {
                'rectangle': self.create_rectangle_node(event, node_size, goal_color_hex),
                'event': event,
                'node_size': node_size,
                'color_hex': goal_color_hex
            }

            self.config['goal_position']['x'] = event.x
            self.config['goal_position']['y'] = event.y

        self.write_json(config_file)

    def paint(self, event):
        self.line_width = self.choose_size_button.get()

        if self.old_x and self.old_x:
            # splinesteps option enhances the smoothness of the line drawn.
            # capstyle sets the shape of the ends of lines. especially relevant when lines meet.
            self.canvas.create_line(self.old_x, self.old_y, event.x, event.y, width=self.line_width,
                                    fill=self.color, capstyle=ROUND, smooth=TRUE, splinesteps=36)

        self.old_x = event.x
        self.old_y = event.y

    def reset(self, event):
        self.old_x, self.old_y = None, None


if __name__ == '__main__':
    Paint()
