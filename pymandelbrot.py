import sys, math, pygame, struct

class Mandelbrot:
	def __init__(self, h, v):
		self.h_res = h
		self.v_res = v
		self.iterations = 30
		self.cursor = False
		self.color_mode = "MONOCHROME"
		self.mode = "MANDELBROT"
		self.exponant = 2
		self.delta_exp = 0.1

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
		wavelength = (self.iterations-val)*(750-380)/self.iterations+380
		attenuation = 0

		if wavelength >= 380 and wavelength <=440:
			attenuation = 0.3 + 0.7 * (wavelength - 380) / (440 - 380)
			r = 0xff * ((-(wavelength-440)/(440-380)) * attenuation)
			b = 0xff * attenuation

		elif wavelength >= 440 and wavelength <= 490:
			g = 0xff * ((wavelength - 440)/(490 - 440))
			b = 0xff

		elif wavelength >= 490 and wavelength <= 510:
			g = 0xff
			b = 0xff * (-(wavelength - 510) / (510 - 490))

		elif wavelength >= 510 and wavelength <= 580:
			r = 0xff * ((wavelength - 510) / (580 - 510))
			g = 0xff

		elif wavelength >= 580 and wavelength <= 645:
			r = 0xff
			g = 0xff * (-(wavelength - 645) / (645 - 580))

		elif wavelength >= 645 and wavelength <= 750:
			attenuation = 0.3 + 0.7 * (750 - wavelength) / (750 - 645)
			r = 0xff * attenuation

		return (r,g,b)

	def compute_number(self, c):
		z = self.start_number
		if self.mode == "JULIA":
			tmp = c
			c = z
			z = tmp
		for i in range(self.iterations):
			if abs(z) >= 2: return i
			try:
				z = z**self.exponant + c
			except:
				z = c
				pass
		return self.iterations

	def render(self):
		if not self.updateNeeded:
			pygame.time.delay(100)
			return
		# Get upper left corner coordinate
		dx = abs(self.left_center.real - self.center.real) / self.h_res * 2
		dy = dx
		up_left = self.left_center.real + (self.left_center.imag+(self.v_res/2)*dy)*1j

		# Compute the image
		for y in range(self.v_res):
			for x in range(self.h_res):
				pt = (up_left.real + x*dx) + (up_left.imag - y*dy)*1j
				self.data[y][x] = self.compute_number(pt)

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

	def writeImg(self):
		bytesPerPix = 4
		width = self.h_res
		height = self.v_res
		f=open("test.bmp",'wb')
		f.write("BM")
		f.write(struct.pack("<i",width*height*bytesPerPix)) #file size
		f.write(bytearray(4)) #file
		f.write(struct.pack("<i",54)) #pixels offset
		f.write(struct.pack("<i",40)) #Header size
		f.write(struct.pack("<i",width)) #width
		f.write(struct.pack("<i",height)) #height
		f.write(struct.pack("<h",1)) # 1
		f.write(struct.pack("<h",8*bytesPerPix)) #bit per pixel
		f.write(struct.pack("<i",0)) #compression
		f.write(struct.pack("<i",0)) #image size
		f.write('\x00'*4) #prefer resol x pix/m
		f.write('\x00'*4) #prefer resol y pix/m
		f.write('\x00'*4) #color used
		f.write('\x00'*4) #color significant

		for y in range(self.v_res):
			for x in range(self.h_res):
				r,g,b,a = self.screen.get_at((x,self.v_res-y-1))
				# a = self.screen.get_at((x,y))
				f.write(struct.pack("<B",r))
				f.write(struct.pack("<B",g))
				f.write(struct.pack("<B",b))
				f.write(struct.pack("<B",a))
				#print(a)
		f.close()

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

	def exp_change(self, direction):
		if direction == "UP":
			self.exponant += self.delta_exp
		elif direction == "DOWN":
			self.exponant -= self.delta_exp
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
		print("Exponant:{}".format(self.exponant))
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
		# Screenshot
		if key[pygame.K_p]: mandel.writeImg()
		# Exponant
		if key[pygame.K_u]: mandel.exp_change("DOWN")
		if key[pygame.K_o]: mandel.exp_change("UP")
		if key[pygame.K_ESCAPE]: return
		mandel.render()

def main():
	witdth, height = (150,150)
	mandel = Mandelbrot(witdth,height)
	loop(mandel)

main()
