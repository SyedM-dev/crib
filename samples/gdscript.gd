# Sample GDScript for syntax highlighting

extends Node2D

# ============================================================
# Constants
# ============================================================
const MAX_HEALTH = 100
const PLAYER_SPEED = 200
const PI_APPROX = 3.14159

# ============================================================
# Exported variables
# ============================================================
@export var player_name: String = "Hero"
@export var is_alive: bool = true

# ============================================================
# Signals
# ============================================================
signal health_changed(new_health)

# ============================================================
# Member variables
# ============================================================
var health: int = MAX_HEALTH
var velocity: Vector2 = Vector2.ZERO
var inventory: Array = []

# ============================================================
# Functions
# ============================================================
func _ready() -> void:
    print("Player ready:", player_name)
    _initialize_inventory()
    set_process(true)

func _process(delta: float) -> void:
    if is_alive:
        _handle_input(delta)
        _check_health()

# Private functions
func _initialize_inventory() -> void:
    inventory.append("Sword")
    inventory.append("Shield")

func _handle_input(delta: float) -> void:
    var direction: Vector2 = Vector2.ZERO
    if Input.is_action_pressed("ui_right"):
        direction.x += 1
    if Input.is_action_pressed("ui_left"):
        direction.x -= 1
    if Input.is_action_pressed("ui_down"):
        direction.y += 1
    if Input.is_action_pressed("ui_up"):
        direction.y -= 1

    velocity = direction.normalized() * PLAYER_SPEED
    position += velocity * delta

func _check_health() -> void:
    if health <= 0:
        is_alive = false
        print("Player is dead!")
    else:
        emit_signal("health_changed", health)

# ============================================================
# Example of class definition inside another script
# ============================================================
class Weapon:
    var name: String
    var damage: int

    func _init(name: String, damage: int):
        self.name = name
        self.damage = damage

    func attack():
        print(name, "attacks for", damage, "damage")
