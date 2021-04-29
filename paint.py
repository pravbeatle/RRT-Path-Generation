
from tkinter import *
from PIL import Image

DEFAULT_BRUSH_SIZE = 6
DEFAULT_COLOR = 'black'
SCREEN_WIDTH = 600
SCREEN_HEIGHT = 600

class Paint():

    def __init__(self):
        self.tk = Tk()

        self.save_button = Button(self.tk, text='Save', command=self.save)
        self.save_button.grid(row=0, column=0)

        self.erase_button = Button(self.tk, text='Clear', command=self.erase)
        self.erase_button.grid(row=0, column=1)

        self.brush_button = Button(self.tk, text='Brush', command=self.brush)
        self.brush_button.grid(row=0, column=2)

        self.choose_size_button = Scale(self.tk, from_=3, to=10, orient=HORIZONTAL)
        self.choose_size_button.grid(row=0, column=3)

        self.canvas = Canvas(self.tk, bg='white', width=SCREEN_WIDTH, height=SCREEN_HEIGHT)
        self.canvas.grid(row=1, columnspan=5)

        self.setup()
        self.tk.mainloop()

    def setup(self):
        self.old_x = None
        self.old_y = None

        self.color = DEFAULT_COLOR

        # set default brush size and line width from it
        self.choose_size_button.set(DEFAULT_BRUSH_SIZE)
        self.line_width = self.choose_size_button.get()

        # set default active button and make it 'sink' to indicate it
        self.active_button = self.brush_button
        self.active_button.config(relief=SUNKEN)

        # <B1-Motion>: mouse is moved with left mouse button held down
        self.canvas.bind('<B1-Motion>', self.paint)
        # <ButtonRelease-1>: left mouse button was released
        self.canvas.bind('<ButtonRelease-1>', self.reset)

    def save(self, filename='map'):
        self.canvas.postscript(file=filename + '.eps')

        image = Image.open(filename + '.eps')
        image.save(filename + '.png', 'png')

        print('Image was saved successfully.')
        quit()

    def erase(self):
        self.canvas.delete('all')

    def brush(self):
        self.activate_button(self.brush_button)

    def activate_button(self, button):
        # raise currently active button and sink newly activated button
        self.active_button.config(relief=RAISED)

        button.config(relief=SUNKEN)
        self.active_button = button

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
