<?php
$host = 'localhost';
$user = 'root';
$pass = 'root'; // oppure '' se non hai password su MAMP
$dbname = 'caregiver';

$conn = new mysqli($host, $user, $pass, $dbname);

if ($conn->connect_error) {
  die("Connessione fallita: " . $conn->connect_error);
}
?>