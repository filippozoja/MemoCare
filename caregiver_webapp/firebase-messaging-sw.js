// firebase-messaging-sw.js

importScripts('https://www.gstatic.com/firebasejs/10.7.1/firebase-app-compat.js');
importScripts('https://www.gstatic.com/firebasejs/10.7.1/firebase-messaging-compat.js');

firebase.initializeApp({
  apiKey: "AIzaSyD50AEdWQzkFt0g4cvrOoFqqh-NBruWqSg",
  authDomain: "titani-webapp-caregiver.firebaseapp.com",
  projectId: "titani-webapp-caregiver",
  messagingSenderId: "319134673644",
  appId: "1:319134673644:web:afac7d2a896f6a5dfd53c9"
});

const messaging = firebase.messaging();

messaging.onBackgroundMessage(function(payload) {
  console.log('[firebase-messaging-sw.js] Messaggio in background ricevuto:', payload);
  const notificationTitle = payload.notification.title;
  const notificationOptions = {
    body: payload.notification.body,
    icon: '/icons/icon-192.png'
  };

  self.registration.showNotification(notificationTitle, notificationOptions);
});
