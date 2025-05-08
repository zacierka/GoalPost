require('dotenv').config();
const express = require('express');
const bodyParser = require('body-parser');
const morgan = require('morgan');
const path = require('path');
const connectToMongo = require('./db');
const Match = require('./models/Match');
// Initialize app
const app = express();
const PORT = process.env.HTTP_PORT || 3000;

// Middleware
app.use(morgan('dev'));
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));

connectToMongo();

// Receive match stats from BakkesMod plugin
app.post('/api/matchstats', async (req, res) => {
    try {
        // console.log(`Received match stats: \n ${JSON.stringify(req.body, null, 2)}`);
        console.log(`Match received: ${req.body.matchId}`);
        
        // Validate request body
        if (!req.body) {
            return res.status(400).json({ error: 'Invalid match data' });
        }

        try {
            const match = new Match(req.body);
            await match.save();
            console.log(`Saved match ${req.body.matchId}`);
            
        } catch (error) {
            console.error(error);
            res.status(400).json({ error: 'Failed to save match' });
        }

        res.status(201).json({ success: true, message: 'Match stats saved successfully' });
    } catch (err) {
        console.error('Error processing match stats:', err);
        res.status(500).json({ error: 'Internal server error' });
    }
});



// Serve the main HTML page
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Start server
async function startServer() {
    app.listen(PORT, () => {
        console.log(`Server running on http://localhost:${PORT}`);
    });
}

startServer();