<?php
// php/get_attivita.php

header('Content-Type: application/json; charset=utf-8');
require_once __DIR__ . '/db.php';
session_start();

// Prendi lo username (prima GET, poi sessione)
$username = '';
if (!empty($_GET['username'])) {
    $username = trim($_GET['username']);
} elseif (!empty($_SESSION['username'])) {
    $username = $_SESSION['username'];
}

if (!$username) {
    // Nessun utente specificato: restituisci array vuoto
    echo json_encode([]);
    exit;
}

// Prepara e esegui query
$stmt = $conn->prepare("
    SELECT
      slot,
      nome,
      ora_inizio,
      durata,
      preavviso,
      dove,
      conchi,
      tema,
      icona,
      data_attivita,
      ultima_modifica
    FROM attivita
    WHERE username = ?
    ORDER BY slot ASC
");
$stmt->bind_param('s', $username);
$stmt->execute();
$result = $stmt->get_result();

// Raccogli i risultati
$activities = [];
while ($row = $result->fetch_assoc()) {
    $activities[] = $row;
}

// Clean up
$stmt->close();
$conn->close();

// Output JSON
echo json_encode($activities);