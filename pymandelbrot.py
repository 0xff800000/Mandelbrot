import sys, math, pygame, time
from multiprocessing import Pool

def compute_number_thread(c):
	x,y,cent,start_number,mode,iterations = c
	z = start_number
	if mode == "JULIA":
		tmp = cent
		cent = z
		z = tmp
	for i in range(iterations):
		if abs(z) >= 2: return (x,y,i)
		z = z*z + cent
	return (x,y,iterations)

class Mandelbrot:
	def __init__(self, h, v):
		self.h_res = h
		self.v_res = v
		self.iterations = 30
		self.cursor = False
		self.color_mode = "MONOCHROME"
		self.mode = "MANDELBROT"

		# Create window
		self.screen = pygame.display.set_mode((h,v))

		# Black screen
		self.screen.fill((0,0,0))

		# Init data
		self.data = []
		for y in range(self.v_res):
			line = []
			for x in range(self.h_res):
				line.append(0)
			self.data.append(line)

		# Init complex
		self.center = 0+0j
		self.left_center = -2+0j
		self.start_number = 0+0j
		self.updateNeeded = True

		# Zoom values
		self.zoom_stat = 1/(self.center.real - self.left_center.real)
		self.max_zoom = 1e100 #278339

	def min_max(self):
		val = []
		for x in self.data:
			val += x
		return (min(val),max(val))

	def map_color(self, val, min, max):
		return val*255 / self.iterations

	def sweep_color(self, val):
		r,g,b = (0,0,0)
		if val == self.iterations: return (r,g,b)
		iter_scale = int(self.iterations / 4)

		# ff,0..ff,0
		if val <= iter_scale:
			r = 0xff
			g = 0xff * val/self.iterations

		# ff..0,ff,0
		elif val <= 2*iter_scale:
			r = 0xff * (1 - val/self.iterations)
			g = 0xff

		# 0,ff,0..ff
		elif val <= 3*iter_scale:
			g = 0xff
			b = 0xff * val/self.iterations

		# 0,ff..0,ff
		elif val <= 4*iter_scale:
			g = 0xff * (1 - val/self.iterations)
			b = 0xff

		# 0,0,ff..0
		else:
			b = 0xff * (1 - val/self.iterations)

		return (r,g,b)

	def compute_number(self, c):
		z = self.start_number
		if self.mode == "JULIA":
			tmp = c
			c = z
			z = tmp
		for i in range(self.iterations):
			if abs(z) >= 2: return i
			z = z*z + c
		return self.iterations


	def render(self):
		if not self.updateNeeded: return
		# Get upper left corner coordinate
		dx = abs(self.left_center.real - self.center.real) / self.h_res * 2
		dy = dx
		up_left = self.left_center.real + (self.left_center.imag+(self.v_res/2)*dy)*1j

		# Compute the image
		pts = []
		p = Pool(10)
		for y in range(self.v_res):
			for x in range(self.h_res):
				nb = (up_left.real + x*dx) + (up_left.imag - y*dy)*1j
				pts.append((x,y, nb,self.start_number,self.mode,self.iterations))
		res = p.map(compute_number_thread, pts)
		p.close()
		for pt in res:
			if(len(pt) == 3):
				x = pt[0]
				y = pt[1]
				val = pt[2]
				self.data[y][x] = val

		# Draw SDL monochrome image
		min_d,max_d = self.min_max()
		for y in range(self.v_res):
			for x in range(self.h_res):
				color = self.map_color(self.data[y][x], min_d, max_d)
				if self.color_mode == "MONOCHROME":
					self.screen.set_at((x,y), (color,color,color))
				else:
					r,g,b = self.sweep_color(self.data[y][x])
					self.screen.set_at((x,y), (r,g,b))

		# Draw cursor
		if self.cursor:
			size = 6
			pt1 = (self.h_res/2,self.v_res/2-size)
			pt2 = (self.h_res/2,self.v_res/2+size)
			pygame.draw.line(self.screen,(0,0xff,0), pt1, pt2)
			pt1 = (self.h_res/2-size,self.v_res/2)
			pt2 = (self.h_res/2+size,self.v_res/2)
			pygame.draw.line(self.screen,(0,0xff,0), pt1, pt2)

		# Render frame
		pygame.display.flip()
		self.updateNeeded = False
		self.debug()

	def set_center(self,pos):
		x,y = pos
		# Get upper left corner
		dx = abs(self.left_center.real - self.center.real) / self.h_res * 2
		dy = dx
		up_left = self.left_center.real + (self.left_center.imag+(self.v_res/2)*dy)*1j
		# Keep difference
		diff = self.center.real - self.left_center.real
		# Set new center
		click = -dx*x+(dy*y)*1j
		click = up_left - click
		print('Click at:{}'.format(click))
		self.center = click
		self.left_center = self.center - diff;
		self.updateNeeded = True

	def shift(self, direction):
		dx = 5*abs(self.left_center.real - self.center.real) / self.h_res
		c = 0
		if direction == "UP":
			c = -dx*1j
		elif direction == "DOWN":
			c = dx*1j
		elif direction == "LEFT":
			c = -dx
		elif direction == "RIGHT":
			c = dx
		self.center += c
		self.left_center += c
		self.updateNeeded = True

	def c_move(self, direction):
		dx = 5*abs(self.left_center.real - self.center.real) / self.h_res
		c = 0
		if direction == "UP":
			c = -dx*1j
		elif direction == "DOWN":
			c = dx*1j
		elif direction == "LEFT":
			c = -dx
		elif direction == "RIGHT":
			c = dx
		self.start_number += c
		self.updateNeeded = True

	def zoom(self, direction):
		dx = abs(self.left_center.real - self.center.real) / self.h_res
		zoom_max = False
		if self.zoom_stat >= self.max_zoom: zoom_max = True

		c = 0
		if direction == "UP":
			c = -100*dx
		elif direction == "DOWN":
			if zoom_max: return
			c = 100*dx
		self.left_center += c
		self.zoom_stat = 1/(self.center.real - self.left_center.real)
		self.updateNeeded = True

	def resol(self, direction):
		if direction =="UP":
			self.iterations += 10
		elif direction == "DOWN":
			self.iterations -= 10
			if self.iterations < 10: self.iterations = 10
		self.updateNeeded = True

	def toggle_cursor(self):
		self.cursor = ~self.cursor
		self.updateNeeded = True

	def cycle_color(self):
		if self.color_mode == "MONOCHROME":
			self.color_mode = "COLOR"
		else:
			self.color_mode = "MONOCHROME"
		self.updateNeeded = True

	def change_mode(self):
		if self.mode == "MANDELBROT":
			self.mode = "JULIA"
		else:
			self.mode = "MANDELBROT"
		self.updateNeeded = True

	def debug(self):
		print("Center:{} ; {}".format(self.center,self.data[int(self.v_res/2)][int(self.h_res/2)]))
		print("Left:{}".format(self.left_center))
		print("Zoom status:{}".format(self.zoom_stat))
		print("Complex number:{}".format(self.start_number))
		print("Iterations:{}".format(self.iterations))
		print("************")

def loop(mandel):
	while True:
		key = pygame.key.get_pressed()
		for event in pygame.event.get():
			if event.type == pygame.QUIT: pygame.quit(); sys.exit()
			# Cursor
			if key[pygame.K_c]: mandel.toggle_cursor()
			# Color mode
			if key[pygame.K_v]: mandel.cycle_color()
			# Change mode
			if key[pygame.K_m]: mandel.change_mode()

			if event.type == pygame.MOUSEBUTTONDOWN:
				x,y = event.pos
				print('Click detected at :{}'.format(event.pos))
				mandel.set_center((x,y))
			# Change complex number
			if key[pygame.K_k]: mandel.c_move("DOWN")
			if key[pygame.K_i]: mandel.c_move("UP")
			if key[pygame.K_j]: mandel.c_move("LEFT")
			if key[pygame.K_l]: mandel.c_move("RIGHT")
			# Zoom
			if key[pygame.K_q]: mandel.zoom("UP")
			if key[pygame.K_e]: mandel.zoom("DOWN")
			# Movement
			if key[pygame.K_w]: mandel.shift("DOWN")
			if key[pygame.K_s]: mandel.shift("UP")
			if key[pygame.K_a]: mandel.shift("LEFT")
			if key[pygame.K_d]: mandel.shift("RIGHT")
			# Resolution
			if key[pygame.K_x]: mandel.resol("UP")
			if key[pygame.K_y]: mandel.resol("DOWN")
		mandel.render()
		#ygame.time.delay(10)

def main():
	witdth, height = (500,500)
	mandel = Mandelbrot(witdth,height)
	loop(mandel)

main()
