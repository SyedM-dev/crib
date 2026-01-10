<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <title>PHP Syntax Stress Test</title>

  <style>
    /* CSS section */
    body {
      font-family: Arial, sans-serif;
      background: #1e1e1e;
      color: #e0e0e0;
    }

    .box {
      border: 1px solid #444;
      padding: 10px;
      margin: 10px;
    }
  </style>

  <script>
    // JS section
    function greet(name) {
      console.log("Hello " + name);
    }

    document.addEventListener("DOMContentLoaded", () => {
      greet("World");
    });
  </script>
</head>

<body>

  <?php
  // Basic variables
  $number = 42;
  $text = "Hello PHP";
  $truth = true;
  $nothing = null;

  // Constants
  define("APP_NAME", "SyntaxTester");

  // Arrays
  $list = [1, 2, 3];
  $assoc = [
    "one" => 1,
    "two" => 2
  ];

  // Function
  function add(int $a, int $b): int
  {
    return $a + $b;
  }

  // Class + methods
  class User
  {
    private string $name;
    public static int $count = 0;

    public function __construct(string $name)
    {
      $this->name = $name;
      self::$count++;
    }

    public function greet(): string
    {
      return "Hello {$this->name}";
    }
  }

  // Object usage
  $user = new User("Alice");
  echo $user->greet();

  // Control flow
  if ($number > 10) {
    echo "Big number";
  } elseif ($number === 10) {
    echo "Exactly ten";
  } else {
    echo "Small number";
  }

  // Loop
  foreach ($list as $item) {
    echo $item;
  }

  // Match expression
  $result = match ($number) {
    1 => "one",
    2 => "two",
    default => "many"
  };

  // Try / catch
  try {
    throw new Exception("Test exception");
  } catch (Exception $e) {
    echo $e->getMessage();
  }

  // Anonymous function
  $double = fn($x) => $x * 2;

  // Nullsafe operator
  $len = $user?->name ? strlen($user->name) : 0;

  // Ternary
  $status = $truth ? "yes" : "no";

  // Include / require
  require_once "config.php";

  // Output
  echo "<div class='box'>";
  echo htmlspecialchars($text);
  echo "</div>";
  ?>

  <script>
    // JS interacting with PHP output
    const phpValue = <?= json_encode($number) ?>;
    console.log("Value from PHP:", phpValue);
  </script>

</body>

</html>
