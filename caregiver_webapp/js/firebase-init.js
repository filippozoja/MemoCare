import { initializeApp } from 'https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js';
import { getMessaging, getToken, onMessage } from 'https://www.gstatic.com/firebasejs/10.7.1/firebase-messaging.js';

const firebaseConfig = {
    apiKey: "AIzaSyD50AEdWQzkFt0g4cvrOoFqqh-NBruWqSg",
    authDomain: "titani-webapp-caregiver.firebaseapp.com",
    projectId: "titani-webapp-caregiver",
    storageBucket: "titani-webapp-caregiver.firebasestorage.app",
    messagingSenderId: "319134673644",
    appId: "1:319134673644:web:afac7d2a896f6a5dfd53c9",
    measurementId: "G-L46VC62RXG"
};

const app = initializeApp(firebaseConfig);
let messaging;
navigator.serviceWorker.register('/caregiver_webapp/firebase-messaging-sw.js')
  .then((registration) => {
    console.log('Service Worker registrato manualmente');
    messaging = getMessaging(app);
    
    Notification.requestPermission().then(permission => {
      if (permission === 'granted') {
        console.log('Permesso notifiche concesso');
        getToken(messaging, {
          vapidKey: 'BH9POY7B58rkUZ9Ifwp3DU8JcJCCvdYEKQGzCbIK6DlDoE1lEYgSjOOdPLZ5XVp6RI7WuAyfpoSHY7k0MECKIP0',
          serviceWorkerRegistration: registration
        }).then((currentToken) => {
          if (currentToken) {
            console.log('Token FCM:', currentToken);
            // Qui puoi inviarlo al tuo server via fetch/AJAX per salvarlo
          } else {
            console.log('Nessun token disponibile');
          }
        }).catch((err) => {
          console.error('Errore nel recupero del token', err);
        });
      } else {
        console.log('Permesso notifiche negato');
      }
    });

    onMessage(messaging, (payload) => {
      console.log('Messaggio ricevuto in foreground:', payload);
      const notificationTitle = payload.notification.title;
      const notificationOptions = {
        body: payload.notification.body,
        icon: '/icons/icon-192.png'
      };
      new Notification(notificationTitle, notificationOptions);
    });
  })
  .catch((err) => {
    console.error('Errore nella registrazione del Service Worker:', err);
  });
