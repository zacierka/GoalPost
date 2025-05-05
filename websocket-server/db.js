const mongoose = require('mongoose');

const URI = process.env.MONGODB_URI; // Replace with your MongoDB URI


const connectToMongo = async () => {
    try {
        await mongoose.connect(URI, {
            useNewUrlParser: true,
            useUnifiedTopology: true,
        });
        console.log('Connected to MongoDB via Mongoose');
    } catch (err) {
        console.error('Mongoose connection error:', err);
    }
};

process.on('SIGINT', async () => {
    await mongoose.connection.close();
    console.log('Mongoose connection closed due to app termination');
    process.exit(0);
});

module.exports = connectToMongo;
