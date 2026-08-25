import React, { useState, useEffect } from 'react';
import { Header } from './components/Header';
import { Sidebar } from './components/Sidebar';
import { Breadcrumbs } from './components/Breadcrumbs';
import { DocContent } from './components/DocContent';
import { TableOfContents } from './components/TableOfContents';
import { SearchModal } from './components/SearchModal';
import { DOCS_ARTICLES } from './data/docsContent';

export const App: React.FC = () => {
  const [activeId, setActiveId] = useState<string>('intro');
  const [isSearchOpen, setIsSearchOpen] = useState<boolean>(false);
  const [isSidebarOpen, setIsSidebarOpen] = useState<boolean>(false);
  const [theme, setTheme] = useState<'dark' | 'light'>('dark');

  // Handle URL hash changes
  useEffect(() => {
    const handleHash = () => {
      const hash = window.location.hash.replace(/^#/, '');
      if (hash && DOCS_ARTICLES[hash]) {
        setActiveId(hash);
      }
    };

    handleHash();
    window.addEventListener('hashchange', handleHash);
    return () => window.removeEventListener('hashchange', handleHash);
  }, []);

  // Prevent body scroll when mobile sidebar is open
  useEffect(() => {
    if (isSidebarOpen && window.innerWidth <= 768) {
      document.body.style.overflow = 'hidden';
    } else {
      document.body.style.overflow = '';
    }
  }, [isSidebarOpen]);

  // Keyboard shortcut listener for search (press '/' or 'Ctrl+K')
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (
        (e.key === '/' && document.activeElement?.tagName !== 'INPUT' && document.activeElement?.tagName !== 'TEXTAREA') ||
        ((e.metaKey || e.ctrlKey) && e.key === 'k')
      ) {
        e.preventDefault();
        setIsSearchOpen(true);
      }
    };

    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);

  // Handle Theme
  useEffect(() => {
    const savedTheme = (localStorage.getItem('meridian_theme') as 'dark' | 'light') || 'dark';
    setTheme(savedTheme);
    document.documentElement.setAttribute('data-theme', savedTheme);
  }, []);

  const toggleTheme = () => {
    const nextTheme = theme === 'dark' ? 'light' : 'dark';
    setTheme(nextTheme);
    document.documentElement.setAttribute('data-theme', nextTheme);
    localStorage.setItem('meridian_theme', nextTheme);
  };

  const handleSelectArticle = (id: string) => {
    if (DOCS_ARTICLES[id]) {
      setActiveId(id);
      window.location.hash = id;
      setIsSidebarOpen(false);
      window.scrollTo({ top: 0, behavior: 'smooth' });
    }
  };

  const currentArticle = DOCS_ARTICLES[activeId] || DOCS_ARTICLES['intro'];

  return (
    <div className="meridian-docs-app">
      <Header
        onOpenSearch={() => setIsSearchOpen(true)}
        onToggleSidebar={() => setIsSidebarOpen(!isSidebarOpen)}
        isSidebarOpen={isSidebarOpen}
        theme={theme}
        onToggleTheme={toggleTheme}
        currentSectionTitle={currentArticle.title}
      />

      <div className="meridian-layout">
        {/* Mobile Backdrop Overlay */}
        <div
          className={`mobile-backdrop ${isSidebarOpen ? 'active' : ''}`}
          onClick={() => setIsSidebarOpen(false)}
          aria-hidden="true"
        />

        <Sidebar
          activeId={activeId}
          onSelect={handleSelectArticle}
          isOpen={isSidebarOpen}
          onClose={() => setIsSidebarOpen(false)}
        />

        <main className="meridian-main-wrapper">
          <div className="meridian-content-container">
            <Breadcrumbs
              category={currentArticle.category}
              title={currentArticle.title}
              onNavigateHome={() => handleSelectArticle('intro')}
            />

            <DocContent
              article={currentArticle}
              onNavigate={handleSelectArticle}
            />
          </div>

          <TableOfContents
            headings={currentArticle.headings}
            onNavigateToContributing={() => handleSelectArticle('proj-contributing')}
          />
        </main>
      </div>

      <SearchModal
        isOpen={isSearchOpen}
        onClose={() => setIsSearchOpen(false)}
        onSelectArticle={handleSelectArticle}
      />
    </div>
  );
};
