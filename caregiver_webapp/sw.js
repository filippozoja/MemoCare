// sw.js

self.addEventListener('install', function(event) {
    console.log('Service Worker installato');
    self.skipWaiting();
  });
  
  self.addEventListener('activate', function(event) {
    console.log('Service Worker attivo');
  });
  
  /*self.addEventListener('fetch', function(event) {
    // Puoi gestire il caching qui se vuoi
  });*/

  console.log("Service Worker attivo - pronto per fetch e notifiche");