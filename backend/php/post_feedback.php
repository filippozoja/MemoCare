<?php
ini_set('display_errors', 1);
error_reporting(E_ALL);
header('Content-Type: application/json');
$data = json_decode(file_get_contents('php://input'), true);
$username = $data['username'] ?? '';
require_once __DIR__ . '/db.php';

if ($conn->connect_error) {
  echo json_encode(["status" => "errore", "msg" => "Connessione fallita"]);
  exit;
}

$stmt = $conn->prepare("INSERT INTO feedback (slot, `timestamp`, username) VALUES (?, ?, ?)");
if (!$stmt) {
  echo json_encode(["status" => "errore", "msg" => $conn->error]);
  exit;
}

$stmt->bind_param("iss", $data['slot'], $data['timestamp'], $username);
$stmt->execute();

if ($stmt->error) {
  echo json_encode(["status" => "errore", "msg" => $stmt->error]);
  exit;
}

echo json_encode(["status" => "ok", "msg" => "Feedback ricevuto"]);
?>