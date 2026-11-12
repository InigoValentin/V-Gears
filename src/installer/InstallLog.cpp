/*
 * Copyright (C) 2022 The V-Gears Team
 *
 * This file is part of V-Gears
 *
 * V-Gears is free software: you can redistribute it and/or modify it under
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.0 (GPLv3) of the License.
 *
 * V-Gears is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "InstallLog.h"

/**
 * Number of messages of each severity that are kept for the summary.
 */
static const size_t kKeptMessages = 200;

/**
 * Stream buffer that passes everything through to the original buffer and sends each complete line to the log.
 */
class InstallLog::Tee : public std::streambuf{

    public:

        Tee(std::ostream& stream, InstallLog& log): stream_(stream), log_(log), original_(stream.rdbuf(this)){}

        ~Tee(){stream_.rdbuf(original_);}

        /**
         * Writes directly to the original stream, without capturing it.
         */
        void WriteOriginal(const std::string& text){original_->sputn(text.data(), text.size());}

    protected:

        int_type overflow(int_type c) override{
            if (c == traits_type::eof()) return traits_type::not_eof(c);
            original_->sputc(static_cast<char>(c));
            if (c == '\n'){
                std::string line;
                line.swap(line_);
                if (!line.empty()) log_.AddCaptured(line);
            }
            else line_ += static_cast<char>(c);
            return c;
        }

        int sync() override{return original_->pubsync();}

    private:

        std::ostream& stream_;
        InstallLog& log_;
        std::streambuf* original_;
        std::string line_;
};

InstallLog::InstallLog(): counts_{0, 0, 0}{}

InstallLog& InstallLog::Get(){
    static InstallLog log;
    return log;
}

void InstallLog::Open(const std::string& path, bool append){
    Close();
    std::lock_guard<std::mutex> lock(mutex_);
    path_ = path;
    file_.open(path, std::ios::out | (append ? std::ios::app : std::ios::trunc));
    for (int i = 0; i < 3; ++ i){
        counts_[i] = 0;
        first_[i].clear();
    }
    last_message_.clear();
    in_step_ = false;
    tee_out_ = std::make_unique<Tee>(std::cout, *this);
    tee_err_ = std::make_unique<Tee>(std::cerr, *this);
}

void InstallLog::Close(){
    // The tees restore the streams on destruction. Destroy them without holding the lock, since they
    // don't use it.
    tee_out_.reset();
    tee_err_.reset();
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) file_.close();
}

void InstallLog::SetSink(Sink sink){
    std::lock_guard<std::mutex> lock(mutex_);
    sink_ = std::move(sink);
}

InstallLog::Level InstallLog::Classify(const std::string& message){
    if (message.compare(0, 7, "[ERROR]") == 0 || message.compare(0, 9, "Exception") == 0) return ERROR_LEVEL;
    if (message.compare(0, 9, "[WARNING]") == 0) return WARNING;
    return INFO;
}

void InstallLog::Add(const std::string& message, bool show){
    const Level level = Classify(message);
    Sink sink;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // Some messages are written twice in a row by the code that produces them.
        if (level != INFO && message == last_message_) return;
        last_message_ = message;
        ++ counts_[level];
        if (first_[level].size() < kKeptMessages) first_[level].push_back(message);
        if (file_.is_open()) file_ << message << '\n' << std::flush;
        if (show) sink = sink_;
    }
    // Echo to the terminal without capturing the message again.
    if (tee_out_) tee_out_->WriteOriginal(message + "\n");
    if (sink) sink(level, message);
}

void InstallLog::AddCaptured(const std::string& line){
    const Level level = Classify(line);
    Sink sink;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (level != INFO && line == last_message_) return;
        if (level != INFO) last_message_ = line;
        ++ counts_[level];
        if (level != INFO && first_[level].size() < kKeptMessages) first_[level].push_back(line);
        if (file_.is_open()) file_ << line << '\n';
        // Only problems are shown in the window; the rest of the captured output is too verbose.
        if (level != INFO) sink = sink_;
    }
    if (sink) sink(level, line);
}

void InstallLog::BeginStep(const std::string& name){
    EndStep();
    std::lock_guard<std::mutex> lock(mutex_);
    step_name_ = name;
    for (int i = 0; i < 3; ++ i) step_start_[i] = counts_[i];
    in_step_ = true;
}

void InstallLog::EndStep(){
    std::string summary;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!in_step_) return;
        in_step_ = false;
        const size_t errors = counts_[ERROR_LEVEL] - step_start_[ERROR_LEVEL];
        const size_t warnings = counts_[WARNING] - step_start_[WARNING];
        if (errors == 0 && warnings == 0) return;
        summary = "Step \"" + step_name_ + "\" finished with " + std::to_string(errors) + " error(s) and "
          + std::to_string(warnings) + " warning(s).";
    }
    // Not prefixed, so it is information and doesn't count as a problem itself.
    Add(summary);
}

size_t InstallLog::Count(Level level) const{
    std::lock_guard<std::mutex> lock(mutex_);
    return counts_[level];
}

std::vector<std::string> InstallLog::First(Level level, size_t max) const{
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> messages = first_[level];
    if (messages.size() > max) messages.resize(max);
    return messages;
}

std::string InstallLog::Path() const{
    std::lock_guard<std::mutex> lock(mutex_);
    return path_;
}
