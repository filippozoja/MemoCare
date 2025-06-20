<?php
header('Content-Type: application/json');
require_once __DIR__ . '/db.php';
if ($conn->connect_error) {
  die(json_encode(["status" => "errore", "msg" => "Connessione fallita"]));
}

$username = $_GET['username'] ?? '';
$stmt = $conn->prepare("SELECT slot, timestamp FROM feedback WHERE username = ? ORDER BY timestamp DESC");
$stmt->bind_param("s", $username);
$stmt->execute();
$result = $stmt->get_result();
$data = [];
while ($row = $result->fetch_assoc()) {
  $data[] = $row;
}

if (empty($data)) {
  echo json_encode(["status" => "vuoto", "msg" => "Nessun feedback trovato"]);
} else {
  echo json_encode(["status" => "ok", "feedback" => $data]);
}
?>