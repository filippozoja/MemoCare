-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost:8889
-- Creato il: Giu 20, 2025 alle 13:01
-- Versione del server: 8.0.40
-- Versione PHP: 8.3.14

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `caregiver`
--

-- --------------------------------------------------------

--
-- Struttura della tabella `attivita`
--

CREATE TABLE `attivita` (
  `id` int UNSIGNED NOT NULL,
  `slot` int NOT NULL,
  `nome` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `ora_inizio` time NOT NULL,
  `durata` int NOT NULL,
  `preavviso` int DEFAULT '0',
  `dove` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `conchi` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `tema` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `icona` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `data_attivita` date DEFAULT NULL,
  `ultima_modifica` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `username` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Dump dei dati per la tabella `attivita`
--

INSERT INTO `attivita` (`id`, `slot`, `nome`, `ora_inizio`, `durata`, `preavviso`, `dove`, `conchi`, `tema`, `icona`, `data_attivita`, `ultima_modifica`, `username`) VALUES
(1, 1, 'Lavarsi i denti', '07:00:00', 30, 0, 'bagno', 'Da solo', 'igiene', '🫧', '2025-05-22', '2025-05-21 23:27:15', 'filippo'),
(2, 2, 'Colazione', '09:00:00', 20, 5, '0', 'Eleonora', 'pasto', '🍴', NULL, '2025-05-21 22:29:04', 'filippo'),
(3, 3, 'Pranzo', '12:00:00', 50, 25, '0', 'Nicola', 'pasto', '🍴', NULL, '2025-05-21 22:38:07', 'filippo'),
(4, 4, 'Pranzo', '00:00:00', 0, 0, '0', 'Sola', '', '', NULL, '2025-05-21 22:43:28', 'filippo'),
(5, 5, 'Aperitivo', '18:00:00', 90, 20, '0', 'Geriatria', 'uscita', '🚶🏻', NULL, '2025-05-21 22:46:42', 'filippo'),
(6, 6, 'Quark', '21:00:00', 150, 30, 'salotto', 'Piero', 'passatempo', '🧩', '2025-05-22', '2025-05-21 23:29:44', 'filippo'),
(7, 1, 'Colazione', '00:00:00', 0, 0, '0', 'Sola', '', '', '2025-05-23', '2025-05-21 22:59:35', 'filippo'),
(8, 1, '', '00:00:00', 0, 0, '0', '', '', '', '2025-05-24', '2025-05-23 11:14:03', 'user1');

-- --------------------------------------------------------

--
-- Struttura della tabella `feedback`
--

CREATE TABLE `feedback` (
  `id` int NOT NULL,
  `slot` int NOT NULL,
  `timestamp` datetime DEFAULT CURRENT_TIMESTAMP,
  `username` varchar(255) COLLATE utf8mb4_unicode_ci DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Dump dei dati per la tabella `feedback`
--

INSERT INTO `feedback` (`id`, `slot`, `timestamp`, `username`) VALUES
(1, 1, '2025-06-19 23:41:35', 'filippo'),
(2, 4, '2025-06-19 23:45:09', 'filippo'),
(3, 3, '2025-06-19 23:45:18', ''),
(4, 6, '2025-06-19 23:46:25', '');

-- --------------------------------------------------------

--
-- Struttura della tabella `utenti`
--

CREATE TABLE `utenti` (
  `id` int NOT NULL,
  `username` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `password_hash` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `email` varchar(100) COLLATE utf8mb4_unicode_ci NOT NULL,
  `telefono` varchar(20) COLLATE utf8mb4_unicode_ci NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

--
-- Dump dei dati per la tabella `utenti`
--

INSERT INTO `utenti` (`id`, `username`, `password_hash`, `email`, `telefono`) VALUES
(1, 'filippo', '$2y$10$sHwtO/g5f0oUWT0tZP4KV.mC3ter2opG6eehup3TymVlclW0PXO1a', '', ''),
(2, 'lorenzo', '$2y$10$HuxhtOpgZZzNp.PO/dw6seGir1F6gyhA3imKnm4uS.PI250rPuTuO', 'l@ll.ll', 'l'),
(3, 'FilippoZ', '$2y$10$1b9eOLq9ZWW38EpwHK9AK.CTPdVgRzeJPgTpd.RBPfEPdcVYDFPHu', 'filippo.zoja@gmail.com', '3663300809'),
(4, 'user1', '$2y$10$Ww2hi80CrqV8psEwU2gYtupLG056zTljBc547iPhQj.f34zylimv.', 'user1@aa.aa', '123');

--
-- Indici per le tabelle scaricate
--

--
-- Indici per le tabelle `attivita`
--
ALTER TABLE `attivita`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `ux_user_slot_date` (`username`,`slot`,`data_attivita`);

--
-- Indici per le tabelle `feedback`
--
ALTER TABLE `feedback`
  ADD PRIMARY KEY (`id`);

--
-- Indici per le tabelle `utenti`
--
ALTER TABLE `utenti`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `username` (`username`);

--
-- AUTO_INCREMENT per le tabelle scaricate
--

--
-- AUTO_INCREMENT per la tabella `attivita`
--
ALTER TABLE `attivita`
  MODIFY `id` int UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=9;

--
-- AUTO_INCREMENT per la tabella `feedback`
--
ALTER TABLE `feedback`
  MODIFY `id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;

--
-- AUTO_INCREMENT per la tabella `utenti`
--
ALTER TABLE `utenti`
  MODIFY `id` int NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
