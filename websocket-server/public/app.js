// Global variables
let mmrChart = null;
let globalStats = null;
let currentPlayer = null;

// DOM elements
const refreshButton = document.getElementById('refreshButton');
const playerSearchInput = document.getElementById('playerSearchInput');
const playerSearchButton = document.getElementById('playerSearchButton');
const playerStatsContainer = document.getElementById('playerStatsContainer');
const playerNotFound = document.getElementById('playerNotFound');

// Event listeners
document.addEventListener('DOMContentLoaded', () => {
  loadGlobalStats();
  
  // Add event listeners
  refreshButton.addEventListener('click', loadGlobalStats);
  playerSearchButton.addEventListener('click', searchPlayer);
  playerSearchInput.addEventListener('keyup', (e) => {
    if (e.key === 'Enter') {
      searchPlayer();
    }
  });
});

// Load global stats
async function loadGlobalStats() {
  try {
    showLoading(refreshButton);
    
    const response = await fetch('/api/stats');
    globalStats = await response.json();
    
    // Update
  } catch {

  }
}