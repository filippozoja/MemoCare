if (!window.attivitaPerSlot) {
  window.attivitaPerSlot = {};
}
function handleSave() {
  const activities = [];
  const username = localStorage.getItem("username") || "";

  for (let i = 1; i <= 6; i++) {
    const nomeEl = document.getElementById(`nome_${i}`);
    const oraOre = document.getElementById(`ora_${i}_ore`);
    const oraMinuti = document.getElementById(`ora_${i}_minuti`);
    const durataEl = document.getElementById(`durata_${i}`);
    const preavvisoEl = document.getElementById(`preavviso_${i}`);

    if (!nomeEl || !oraOre || !oraMinuti || !durataEl || !preavvisoEl) continue;

    const nome = nomeEl.value;
    const ora = `${oraOre.value || "00"}:${oraMinuti.value || "00"}`;
    const durata = parseInt(durataEl.value);
    const preavviso = parseInt(preavvisoEl.value);

    // Aggiunta solo se tutti i campi hanno valore valido
    if (nome && !isNaN(durata) && !isNaN(preavviso)) {
      activities.push({
        slot: i,
        nome: nome,
        ora_inizio: ora,
        durata: durata,
        preavviso: preavviso,
        username: username
      });
    }
  }

  // Controllo: almeno una attività valida
  if (activities.length === 0) {
    alert("Nessuna attività valida da salvare.");
    return;
  }

  // Salvataggio
  fetch("php/post_attivita.php", {
    method: "POST",
    headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify(activities)
  })
    .then((res) => res.json())
    .then((data) => {
      if (data.status === "ok") {
        alert("Configurazione salvata con successo!");
      } else {
        alert("Errore nel salvataggio:\n" + JSON.stringify(data));
      }
    })
    .catch(async (err) => {
      console.error("Errore di rete o sintassi:", err);
      alert("Errore di rete.");
    });
}

fetch(`php/get_attivita.php?username=${encodeURIComponent(localStorage.getItem('username'))}`)
  .then(res => res.json())
  .then(data => {
    const tbody = document.querySelector("#attivitaTable tbody");
    if (tbody) {
      data.forEach(attivita => {
        attivitaPerSlot[attivita.slot] = attivita.nome;
        const row = `<tr>
          <td data-label="Slot">${attivita.slot}</td>
          <td data-label="Nome">${attivita.nome}</td>
          <td data-label="Ora Inizio">${attivita.ora_inizio}</td>
          <td data-label="Durata">${attivita.durata} min</td>
          <td data-label="Preavviso">${attivita.preavviso} min</td>
        </tr>`;
        tbody.innerHTML += row;
      });
    }
  })
  .catch(err => {
    console.error("Errore nel caricamento attività:", err);
    alert("Errore nel caricamento delle attività.");
  }); 

document.addEventListener("DOMContentLoaded", () => {
  for (let i = 1; i <= 6; i++) {
    const btn = document.getElementById(`btn_feedback_${i}`);
    if (btn) {
      btn.onclick = () => {
        fetch("php/post_feedback.php", {
          method: "POST",
          headers: {
            "Content-Type": "application/json"
          },
          body: JSON.stringify({
            slot: i,
            username: localStorage.getItem('username'),
            timestamp: new Date().toISOString().slice(0, 19).replace("T", " ")
          })
        })
          .then(response => response.json())
          .then(response => {
            alert(`Feedback per slot ${i} inviato:\n${JSON.stringify(response)}`);
            const attivita = attivitaPerSlot[i] || `slot ${i}`;
            if (Notification.permission === "granted") {
              const titolo = `Attività completata: ${attivita}`;
              const corpo = `Hai completato l'attività \"${attivita}\" nello slot ${i}.`;
              const notifica = new Notification(titolo, {
                body: corpo,
                icon: "icons/icon-192.png",
                vibrate: [200, 100, 200],
                data: {
                  url: `visualizza.html?slot=${i}`
                }
              });
              notifica.onclick = function(event) {
                event.preventDefault();
                window.open(notifica.data.url, "_blank");
              };
            }
          });
      };
    }
  }
});