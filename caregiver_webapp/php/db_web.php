<?php
//$host = 'localhost';
$user = 'xvfilipp_titani';
$pass = 'xvfilipp_titani'; // oppure '' se non hai password su MAMP
$dbname = 'xvfilipp_titani';

$conn = new mysqli($host, $user, $pass, $dbname);

if ($conn->connect_error) {
  die("Connessione fallita: " . $conn->connect_error);
}
?>