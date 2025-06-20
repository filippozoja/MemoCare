<?php
// register.php
require_once "db.php";

if ($_SERVER["REQUEST_METHOD"] === "POST") {
  $username = trim($_POST['username'] ?? '');
  $password = trim($_POST['password'] ?? '');
  $email = trim($_POST['email'] ?? '');
  $telefono = trim($_POST['telefono'] ?? '');

  if (!$username || !$password || !$email || !$telefono) {
    exit("Compila tutti i campi.");
  }

  // Verifica se l'utente esiste già
  $stmt = $conn->prepare("SELECT id FROM utenti WHERE username = ?");
  $stmt->bind_param("s", $username);
  $stmt->execute();
  $stmt->store_result();

  if ($stmt->num_rows > 0) {
    $stmt->close();
    exit("Username già esistente.");
  }
  $stmt->close();

  // Cripta la password e salva
  $hash = password_hash($password, PASSWORD_DEFAULT);
  $stmt = $conn->prepare("INSERT INTO utenti (username, password_hash, email, telefono) VALUES (?, ?, ?, ?)");
  $stmt->bind_param("ssss", $username, $hash, $email, $telefono);

  if ($stmt->execute()) {
    header("Location: ../login.html?registrazione=successo");
    exit;
  } else {
    echo "Errore durante la registrazione.";
  }

  $stmt->close();
  $conn->close();
} else {
  header("Location: ../register.html");
  exit;
}