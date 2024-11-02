import tkinter as tk
from tkinter import messagebox, simpledialog, filedialog
import json
import math

class LevelEditor:
	def __init__(self, master):
		self.level_data = {
			"files_required": 0,
			"name": "new_level",
			"author_time": 0.0,
			"start_position": [0, 0],
			"start_angle": 0,
			"pickups": [],
			"walls": [],
			"zones": [],
			"on_unlock_dialog": 0,
		}

		self.master = master
		self.canvas = tk.Canvas(master, width=1200, height=600, bg="white")
		self.canvas.pack()

		self.offset_x, self.offset_y = 600, 300
		self.drag_data = {"item": None, "type": None, "index": None}
		self.selected_wall = None
		self.selected_zone = None
		self.selected_pickup = None
		self.selected_point_index = None
		self.grid_enabled = True
		self.grid_size = 10

		self.canvas.bind("<ButtonPress-1>", self.on_left_click)
		self.canvas.bind("<ButtonPress-3>", self.on_right_click)
		self.canvas.bind("<B1-Motion>", self.on_mouse_drag)
		self.canvas.bind("<B3-Motion>", self.on_pan_canvas)
		self.canvas.bind("<ButtonRelease-1>", self.on_mouse_release)

		self.canvas.bind('<Key-1>', lambda event: self.add_point_before())
		self.canvas.bind('<Key-2>', lambda event: self.add_point_after())
		self.canvas.bind('<Key-d>', lambda event: self.remove_point())
		self.canvas.bind('<Key-g>', lambda event: self.set_grid_size())
		self.canvas.bind('<Key-t>', lambda event: self.toggle_grid())
		self.canvas.bind('<Key-w>', lambda event: self.add_wall())
		self.canvas.bind('<Key-z>', lambda event: self.add_zone())
		self.canvas.bind('<Key-c>', lambda event: self.add_pickup())

		file_frame = tk.Frame(master)
		file_frame.pack(side=tk.LEFT, anchor="w", padx=5, pady=5)

		add_elements_frame = tk.Frame(master)
		add_elements_frame.pack(side=tk.LEFT, anchor="w", padx=5, pady=5)

		grid_control_frame = tk.Frame(master)
		grid_control_frame.pack(side=tk.LEFT, anchor="w", padx=5, pady=5)

		edit_properties_frame = tk.Frame(master)
		edit_properties_frame.pack(side=tk.LEFT, anchor="w", padx=5, pady=5)

		tk.Button(file_frame, text="Save Level", command=self.save_level).pack(side=tk.TOP, fill=tk.X)
		tk.Button(file_frame, text="Load Level", command=self.load_level).pack(side=tk.TOP, fill=tk.X)
		tk.Button(file_frame, text="Set Start Angle", command=self.set_start_angle).pack(side=tk.TOP, fill=tk.X)

		tk.Button(add_elements_frame, text="Add Pickup", command=self.add_pickup).pack(side=tk.TOP, fill=tk.X)
		tk.Button(add_elements_frame, text="Add Wall", command=self.add_wall).pack(side=tk.TOP, fill=tk.X)
		tk.Button(add_elements_frame, text="Add Zone", command=self.add_zone).pack(side=tk.TOP, fill=tk.X)
		tk.Button(add_elements_frame, text="Add Point Before", command=self.add_point_before).pack(side=tk.TOP, fill=tk.X)
		tk.Button(add_elements_frame, text="Add Point After", command=self.add_point_after).pack(side=tk.TOP, fill=tk.X)
		tk.Button(add_elements_frame, text="Remove Point", command=self.remove_point).pack(side=tk.TOP, fill=tk.X)

		tk.Button(grid_control_frame, text="Toggle Grid", command=self.toggle_grid).pack(side=tk.TOP, fill=tk.X)
		tk.Button(grid_control_frame, text="Set Grid Size", command=self.set_grid_size).pack(side=tk.TOP, fill=tk.X)

		tk.Button(edit_properties_frame, text="Edit Zone Kind", command=self.edit_zone_kind).pack(side=tk.TOP, fill=tk.X)
		tk.Button(edit_properties_frame, text="Edit Zone Value", command=self.edit_zone_value).pack(side=tk.TOP, fill=tk.X)
		tk.Button(edit_properties_frame, text="Edit Zone Power", command=self.edit_zone_power).pack(side=tk.TOP, fill=tk.X)
		tk.Button(edit_properties_frame, text="Edit Wall Kind", command=self.edit_wall_kind).pack(side=tk.TOP, fill=tk.X)
		tk.Button(edit_properties_frame, text="Edit Wall Key ID", command=self.edit_wall_key_id).pack(side=tk.TOP, fill=tk.X)
		tk.Button(edit_properties_frame, text="Edit Pickup Kind", command=self.edit_pickup_kind).pack(side=tk.TOP, fill=tk.X)
		tk.Button(edit_properties_frame, text="Edit Pickup ID", command=self.edit_pickup_id).pack(side=tk.TOP, fill=tk.X)

		self.draw_level()


		self.canvas.focus_set()  # Add this line in __init__

	def edit_zone_kind(self):
		if self.selected_zone is not None:
			zone = self.level_data["zones"][self.selected_zone]
			kind = simpledialog.askinteger("Zone Kind", "Enter kind (0: End, 1: DialogTrigger, 2: OneWay, 3: Danger):", initialvalue=zone["kind"])
			if kind in [0, 1, 2, 3]:
				zone["kind"] = kind
				self.draw_level()
		else:
			messagebox.showinfo("No Zone Selected", "Please select a zone to edit.")

	def edit_zone_value(self):
		if self.selected_zone is not None:
			zone = self.level_data["zones"][self.selected_zone]
			if zone["kind"] == 1:  # DialogTrigger
				dialog_index = simpledialog.askinteger("Dialog Index", "Enter dialog index:", initialvalue=zone.get("value", 0))
				zone["value"] = dialog_index
			elif zone["kind"] == 2:  # OneWay
				one_way_angle = simpledialog.askfloat("One-Way Angle", "Enter one-way angle:", initialvalue=zone.get("value", 0.0))
				zone["value"] = one_way_angle
			self.draw_level()
		else:
			messagebox.showinfo("No Zone Selected", "Please select a zone to edit.")

	def edit_zone_power(self):
		if self.selected_zone is not None:
			zone = self.level_data["zones"][self.selected_zone]
			power = simpledialog.askfloat("Zone Power", "Enter power value:", initialvalue=zone.get("power", 0.0))
			zone["power"] = power
			self.draw_level()
		else:
			messagebox.showinfo("No Zone Selected", "Please select a zone to edit.")

	def edit_wall_kind(self):
		if self.selected_wall is not None:
			wall = self.level_data["walls"][self.selected_wall]
			kind = simpledialog.askinteger("Wall Kind", "Enter kind (0 for Wall, 1 for Door):", initialvalue=wall.get("kind", 0))
			if kind in [0, 1]:
				wall["kind"] = kind
				self.draw_level()
		else:
			messagebox.showinfo("No Wall Selected", "Please select a wall to edit.")

	def edit_wall_key_id(self):
		if self.selected_wall is not None:
			wall = self.level_data["walls"][self.selected_wall]
			if wall.get("kind", 0) == 1:  # Only if the wall is a door
				key_id = simpledialog.askinteger("Key ID", "Enter key ID:", initialvalue=wall.get("key_id", 0))
				wall["key_id"] = key_id
				self.draw_level()
			else:
				messagebox.showinfo("Invalid Wall Type", "The selected wall is not a door.")
		else:
			messagebox.showinfo("No Wall Selected", "Please select a wall to edit.")


	def edit_pickup_kind(self):
		if self.selected_pickup is not None:
			pickup = self.level_data["pickups"][self.selected_pickup]
			kind = simpledialog.askinteger("Pickup Kind", "Enter kind (0 for Key, 1 for File):", initialvalue=pickup["kind"])
			if kind in [0, 1]:
				pickup["kind"] = kind
				self.draw_level()
		else:
			messagebox.showinfo("No Pickup Selected", "Please select a pickup to edit.")

	def edit_pickup_id(self):
		if self.selected_pickup is not None:
			pickup = self.level_data["pickups"][self.selected_pickup]
			pickup_id = simpledialog.askinteger("Pickup ID", "Enter new item ID:", initialvalue=pickup.get("id", 0))
			pickup["id"] = pickup_id
			self.draw_level()
		else:
			messagebox.showinfo("No Pickup Selected", "Please select a pickup to edit.")

	def set_start_angle(self):
		angle = simpledialog.askfloat("Set Start Angle", "Enter angle in degrees:", initialvalue=self.level_data["start_angle"])
		if angle is not None:
			self.level_data["start_angle"] = angle
			self.draw_level()

	def draw_level(self):
		self.canvas.delete("all")
		self.draw_origin()
		self.draw_start_position()
		
		for pickup in self.level_data["pickups"]:
			color = "yellow" if self.selected_pickup is not None and pickup["id"] == self.selected_pickup else "green"
			self.canvas.create_oval(
				pickup["x"] - 5 + self.offset_x, pickup["y"] - 5 + self.offset_y, 
				pickup["x"] + 5 + self.offset_x, pickup["y"] + 5 + self.offset_y, 
				fill=color, outline="black", tags=(f"pickup_{pickup['id']}", "movable")
			)

		for i, wall in enumerate(self.level_data["walls"]):
			points = [(x + self.offset_x, y + self.offset_y) for x, y in wall["points"]]
			for j in range(len(points) - 1):
				self.canvas.create_line(points[j], points[j+1], fill="black", tags=("wall", f"wall_{i}"))
				for k in range(len(points)):
					point_color = "blue" if self.selected_wall == i and k == self.selected_point_index else "black"
					self.canvas.create_oval(
						points[k][0] - 3, points[k][1] - 3, points[k][0] + 3, points[k][1] + 3,
						fill=point_color, tags=("wall_point", f"wall_{i}_point_{k}", "movable")
					)


		for i, zone in enumerate(self.level_data["zones"]):
			zone_points = [(x + self.offset_x, y + self.offset_y) for x, y in zone["points"]]
			self.canvas.create_polygon(zone_points, outline="red", fill="", tags=("zone", f"zone_{i}"))
			for j, (x, y) in enumerate(zone_points):
				point_color = "red" if self.selected_zone == i and j == self.selected_point_index else "black"
				self.canvas.create_oval(
					x - 3, y - 3, x + 3, y + 3, fill=point_color, tags=("zone_point", f"zone_{i}_point_{j}", "movable")
				)

	def draw_origin(self):
		self.canvas.create_line(self.offset_x - 10, self.offset_y, self.offset_x + 10, self.offset_y, fill="gray")
		self.canvas.create_line(self.offset_x, self.offset_y - 10, self.offset_x, self.offset_y + 10, fill="gray")

	def draw_start_position(self):
		x, y = self.level_data["start_position"]
		angle = self.level_data["start_angle"]
		self.canvas.create_oval(x - 5 + self.offset_x, y - 5 + self.offset_y, 
								x + 5 + self.offset_x, y + 5 + self.offset_y, 
								fill="blue", outline="black", tags=("start", "movable"))
		line_length = 20
		end_x = x + line_length * math.cos(math.radians(angle))
		end_y = y + line_length * math.sin(math.radians(angle))
		self.canvas.create_line(x + self.offset_x, y + self.offset_y, end_x + self.offset_x, end_y + self.offset_y, fill="blue")

	def toggle_grid(self):
		self.grid_enabled = not self.grid_enabled
		messagebox.showinfo("Grid Snapping", f"Grid snapping {'enabled' if self.grid_enabled else 'disabled'}")
		self.draw_level()

	def set_grid_size(self):
		size = simpledialog.askinteger("Set Grid Size", "Enter grid size:", initialvalue=self.grid_size)
		if size and size > 0:
			self.grid_size = size
			self.draw_level()

	def on_right_click(self, event):
		self.drag_data = {"item": None, "type": "pan", "start_x": event.x, "start_y": event.y}


	def on_left_click(self, event):
		item = self.canvas.find_closest(event.x, event.y)
		tags = self.canvas.gettags(item)

		self.selected_wall = None
		self.selected_zone = None
		self.selected_pickup = None
		self.selected_point_index = None

		start_x, start_y = self.level_data["start_position"]
		click_x, click_y = event.x - self.offset_x, event.y - self.offset_y
		if abs(click_x - start_x) < 10 and abs(click_y - start_y) < 10:
			self.selected_point_index = "start_position"
		elif any("pickup" in tag for tag in tags):	
			self.selected_pickup = int(tags[0].split("_")[1])
		elif any("wall_point" in tag for tag in tags):
			self.selected_wall, self.selected_point_index = self.extract_index(tags)
		elif any("zone_point" in tag for tag in tags):
			self.selected_zone, self.selected_point_index = self.extract_index(tags)
		else:
			print("Nothing selected")

		self.draw_level()

	def on_mouse_drag(self, event):
		if self.grid_enabled:
			snapped_x = round((event.x - self.offset_x) / self.grid_size) * self.grid_size
			snapped_y = round((event.y - self.offset_y) / self.grid_size) * self.grid_size
		else:
			snapped_x, snapped_y = event.x - self.offset_x, event.y - self.offset_y

		if self.selected_point_index == "start_position":
			self.level_data["start_position"] = [snapped_x, snapped_y]
		elif self.selected_pickup is not None:
			pickup = self.level_data["pickups"][self.selected_pickup]
			pickup["x"], pickup["y"] = snapped_x, snapped_y
		elif self.selected_wall is not None:
			self.level_data["walls"][self.selected_wall]["points"][self.selected_point_index] = [snapped_x, snapped_y]
		elif self.selected_zone is not None:
			self.level_data["zones"][self.selected_zone]["points"][self.selected_point_index] = [snapped_x, snapped_y]
		self.draw_level()


	def save_level(self):
		filename = filedialog.asksaveasfilename(defaultextension=".json", filetypes=[("JSON files", "*.json")])
		if filename:
			with open(filename, "w") as f:
				json.dump(self.level_data, f, indent=4)
			messagebox.showinfo("Saved", f"Level saved to {filename}")

	def load_level(self):
		filename = filedialog.askopenfilename(defaultextension=".json", filetypes=[("JSON files", "*.json")])
		if filename:
			with open(filename, "r") as f:
				self.level_data = json.load(f)
			self.draw_level()

	def add_pickup(self):
		new_x, new_y = 10, 10
		pickup_id = len(self.level_data["pickups"])
		self.level_data["pickups"].append({"kind": 0, "id": pickup_id, "x": new_x, "y": new_y})
		self.draw_level()

	def add_wall(self):
		start_x, start_y = 10, 10
		self.level_data["walls"].append({"kind": 0, "points": [[start_x, start_y], [start_x + 50, start_y + 50]], "key_id": 0})
		self.draw_level()

	def add_zone(self):
		start_x, start_y = 10, 10
		self.level_data["zones"].append({"kind": 0, "points": [[start_x, start_y], [start_x + 50, start_y], [start_x + 50, start_y + 50], [start_x, start_y + 50]], "value": 0})
		self.draw_level()

	def add_point_before(self):
		if self.selected_wall is not None:
			point = self.level_data["walls"][self.selected_wall]["points"][self.selected_point_index]
			offset_point = [point[0] - 10, point[1] - 10]
			self.level_data["walls"][self.selected_wall]["points"].insert(self.selected_point_index, offset_point)
		elif self.selected_zone is not None:
			point = self.level_data["zones"][self.selected_zone]["points"][self.selected_point_index]
			offset_point = [point[0] - 10, point[1] - 10]
			self.level_data["zones"][self.selected_zone]["points"].insert(self.selected_point_index, offset_point)
		self.draw_level()

	def add_point_after(self):
		if self.selected_wall is not None:
			point = self.level_data["walls"][self.selected_wall]["points"][self.selected_point_index]
			offset_point = [point[0] + 10, point[1] + 10]
			self.level_data["walls"][self.selected_wall]["points"].insert(self.selected_point_index + 1, offset_point)
		elif self.selected_zone is not None:
			point = self.level_data["zones"][self.selected_zone]["points"][self.selected_point_index]
			offset_point = [point[0] + 10, point[1] + 10]
			self.level_data["zones"][self.selected_zone]["points"].insert(self.selected_point_index + 1, offset_point)
		self.draw_level()

	def extract_index(self, tags):
		for tag in tags:
			if "wall" in tag and "point" in tag:
				parts = tag.split("_")
				if len(parts) >= 3 and parts[1].isdigit() and parts[2] == "point" and parts[3].isdigit():
					wall_index = int(parts[1])
					point_index = int(parts[3])
					return wall_index, point_index
			elif "zone" in tag and "point" in tag:
				parts = tag.split("_")
				if len(parts) >= 3 and parts[1].isdigit() and parts[2] == "point" and parts[3].isdigit():
					zone_index = int(parts[1])
					point_index = int(parts[3])
					return zone_index, point_index
		return None, None



	def on_pan_canvas(self, event):
		dx = event.x - self.drag_data["start_x"]
		dy = event.y - self.drag_data["start_y"]
		self.offset_x += dx
		self.offset_y += dy
		self.drag_data["start_x"] = event.x
		self.drag_data["start_y"] = event.y
		self.draw_level()
	def on_mouse_release(self, event):
		self.drag_data = {"item": None, "type": None, "index": None}

	def remove_point(self):
		"""Remove the selected point from a wall or zone, and remove the wall or zone if all points are removed."""
		if self.selected_wall is not None and self.selected_point_index is not None:
			points = self.level_data["walls"][self.selected_wall]["points"]
			if len(points) > 1:
				points.pop(self.selected_point_index)
				self.selected_point_index = None
			else:
				self.level_data["walls"].pop(self.selected_wall)
				self.selected_wall = None
				self.selected_point_index = None

		elif self.selected_zone is not None and self.selected_point_index is not None:
			points = self.level_data["zones"][self.selected_zone]["points"]
			if len(points) > 1:
				points.pop(self.selected_point_index)
				self.selected_point_index = None
			else:
				self.level_data["zones"].pop(self.selected_zone)
				self.selected_zone = None
				self.selected_point_index = None

		elif self.selected_pickup is not None:
			self.level_data["pickups"].pop(self.selected_pickup)
			self.selected_pickup = None

		self.draw_level()


if __name__ == "__main__":
	root = tk.Tk()
	root.title("Level Editor")
	editor = LevelEditor(root)
	root.mainloop()
			
