<?php
// login.php
session_start();
require_once "db.php"; // contiene connessione $conn

if ($_SERVER["REQUEST_METHOD"] === "POST") {
  $username = $_POST['username'] ?? '';
  $password = $_POST['password'] ?? '';

  if ($username && $password) {
    $stmt = $conn->prepare("SELECT id, password_hash, email, telefono FROM utenti WHERE username = ?");
    $stmt->bind_param("s", $username);
    $stmt->execute();
    $result = $stmt->get_result();

    if ($result && $result->num_rows === 1) {
      $user = $result->fetch_assoc();
      if (password_verify($password, $user['password_hash'])) {
        $_SESSION['user_id'] = $user['id'];
        $_SESSION['username'] = $username;
        $_SESSION['loggedin'] = true;
        $_SESSION['email'] = $user['email'];
        $_SESSION['telefono'] = $user['telefono'];
        header('Content-Type: application/json');
        echo json_encode([
          "status" => "success",
          "username" => $_SESSION['username'],
          "user_id" => $_SESSION['user_id'],
          "email" => $_SESSION['email'],
          "telefono" => $_SESSION['telefono']
        ]);
        exit;
      } else {
        echo "Password errata.";
      }
    } else {
      echo "Utente non trovato.";
    }
    $stmt->close();
  } else {
    echo "Compila tutti i campi.";
  }
} else {
  header("Location: ../login.html");
  exit;
}
