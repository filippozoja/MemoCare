<?php
// php/post_attivita.php

header('Content-Type: application/json');
require_once __DIR__ . '/db.php';

// Legge il payload JSON
$payload = json_decode(file_get_contents('php://input'), true);
if (!$payload) {
    echo json_encode(['status'=>'errore','msg'=>'JSON non valido']);
    exit;
}

// Se arriva un singolo oggetto (modifica_attività), trasformalo in array
$activities = isset($payload[0]) ? $payload : [$payload];

// Prepara la query: INSERT sempre in nuova riga, senza overwrite
$sql = <<<SQL
INSERT INTO attivita
  (username, slot, nome, ora_inizio, durata, preavviso, dove, conchi, tema, icona, data_attivita, ultima_modifica)
VALUES
  (?,       ?,    ?,    ?,         ?,       ?,         ?,    ?,      ?,     ?,     DATE_ADD(CURDATE(), INTERVAL 1 DAY), NOW())
SQL;

$stmt = $conn->prepare($sql);
if (!$stmt) {
    echo json_encode(['status'=>'errore','msg'=>$conn->error]);
    exit;
}

foreach ($activities as $act) {
    // Mappatura campi del JSON a colonne
    $username      = $act['username']       ?? '';
    $slot          = intval($act['slot']      ?? 0);
    // nel payload di modifica_attività.html usi "attivita", nel payload di aggiungi_attività.html "nome"
    $nome          = $act['attivita']       ?? ($act['nome'] ?? '');
    $ora_inizio    = $act['orario_inizio']  ?? ($act['ora_inizio'] ?? '');
    $durata        = intval($act['durata']    ?? 0);
    $preavviso     = intval($act['preavviso'] ?? 0);
    $dove          = $act['dove']            ?? '';
    $conchi        = $act['conchi']          ?? '';
    $tema          = $act['tema']            ?? '';
    $icona         = $act['icona']           ?? '';

    $stmt->bind_param(
        'sissiiisss',
        $username,
        $slot,
        $nome,
        $ora_inizio,
        $durata,
        $preavviso,
        $dove,
        $conchi,
        $tema,
        $icona
    );
    if (!$stmt->execute()) {
        echo json_encode(['status'=>'errore','msg'=>$stmt->error]);
        exit;
    }
}

echo json_encode(['status'=>'ok']);