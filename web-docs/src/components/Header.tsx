import React from 'react';
import { Search, Moon, Sun, Menu, X, Globe, ChevronDown } from 'lucide-react';

interface HeaderProps {
  onOpenSearch: () => void;
  onToggleSidebar: () => void;
  isSidebarOpen: boolean;
  theme: 'dark' | 'light';
  onToggleTheme: () => void;
  onNavigate: (id: string) => void;
}

export const Header: React.FC<HeaderProps> = ({
  onOpenSearch,
  onToggleSidebar,
  isSidebarOpen,
  theme,
  onToggleTheme,
  onNavigate,
}) => {
  return (
    <header className="meridian-header">
      <div className="header-left">
        <button
          className="mobile-menu-btn"
          onClick={onToggleSidebar}
          aria-label="Toggle navigation"
        >
          {isSidebarOpen ? <X size={18} /> : <Menu size={18} />}
        </button>

        <a href="#intro" onClick={() => onNavigate('intro')} className="brand-logo" title="Meridian Shell Documentation">
          {/* Meridian Shell Logo matching screenshot */}
          <div className="logo-badge">
            <svg viewBox="0 0 100 100" width="28" height="28" className="logo-svg">
              <circle cx="50" cy="50" r="44" fill="none" stroke="#00A8B5" strokeWidth="6" />
              <path
                d="M 28 68 L 28 34 L 50 56 L 72 34 L 72 68"
                fill="none"
                stroke="#00A8B5"
                strokeWidth="7"
                strokeLinecap="round"
                strokeLinejoin="round"
              />
              <path
                d="M 20 54 Q 50 20 80 54"
                fill="none"
                stroke="#00A8B5"
                strokeWidth="3.5"
                strokeDasharray="4 4"
                opacity="0.85"
              />
            </svg>
          </div>
          <div className="brand-text-block">
            <span className="brand-name">MERIDIAN SHELL</span>
            <span className="brand-tag">DOCS</span>
          </div>
        </a>
      </div>

      <div className="header-right">
        <nav className="header-nav-links">
          <button onClick={() => onNavigate('ref-cli')} className="header-nav-link">
            API Docs
          </button>
          <button onClick={() => onNavigate('proj-changelog')} className="header-nav-link">
            Release Notes
          </button>
          <button onClick={() => onNavigate('quickstart')} className="header-nav-link">
            Getting Started
          </button>
        </nav>

        <div className="header-lang-wrapper">
          <button className="header-lang-btn" aria-label="Select Language">
            <Globe size={14} />
            <span>English</span>
            <ChevronDown size={12} />
          </button>
        </div>

        <button className="header-search-btn" onClick={onOpenSearch} aria-label="Search documentation">
          <Search size={14} className="search-icon" />
          <span className="search-placeholder">Search</span>
          <kbd className="search-kbd">Ctrl K</kbd>
        </button>

        <button
          onClick={onToggleTheme}
          className="header-theme-btn"
          title={`Switch to ${theme === 'dark' ? 'light' : 'dark'} mode`}
          aria-label="Toggle theme"
        >
          {theme === 'dark' ? <Moon size={16} /> : <Sun size={16} />}
        </button>
      </div>
    </header>
  );
};
